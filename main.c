#include "hotdraw.h"
#include <core.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>
FILE *fdopen(int fd, const char *mode);

#include <linux/time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);

#include <raylib.h>

#define DL_PATH "libhotfile.so"

Color htoc(n32 hex) { /* 0x RR GG BB AA */
	Color color = {0};
	color.r = hex >> 24;
	color.g = hex >> 16;
	color.b = hex >> 8;
	color.a = hex;
	return color;
}

/* 4x3  200 800x600  80 320x240  50 200x150 */
/* 3x2  200 600x400  80 360x240  50 150x100 */
const n16 WINDOW_WIDTH = 1000;
const n16 WINDOW_HEIGHT = 600;

void show_pending_screen(void) {
	struct timespec duration = {0};
	struct timespec rem = {0};

	duration.tv_nsec = 200 * 1000000; /* 500ms */

	BeginDrawing();
	ClearBackground(htoc(0xFF0000FF));
	DrawRectangle(0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT, htoc(0x00FFFFFF));
	EndDrawing();

	nanosleep(&duration, &rem);
	/* printf("Sleeped for %lums\n", (duration.tv_nsec - rem.tv_nsec) / 1000000); */
}

void parsedump(char* filepath, MEvents* events) {
	String event_buffer = {0};
	MEvent event = {0};
	enum { PR_FILE, PR_TYPE, PR_INPUT, PR_OUTPUT } state = PR_FILE;
	n64 i;
	char c;

	FILE* file = fopen(filepath, "r");
	assert(file);

	for(i = 0; (c = getc(file)) != EOF; ++i) {
		switch(c) {
			case '\n': {
				n32 data;

				string_nterm(&event_buffer);

				errno = 0;
				data = strtol(event_buffer.chars, NULL, 16);
 				assert(!errno);

				switch(event.type) {
					case MALLOC:  event.as.malloc.rptr.raw = data; break;
					case CALLOC:  event.as.calloc.rptr.raw = data; break;
					case REALLOC: event.as.realloc.rptr.raw = data; break;
					case FREE:    event.as.free.ptr.raw = data; break;
					default: assert(0);
				}
				arr_append(events, event);
				string_clear(&event_buffer);
				state = PR_FILE;
				break;
			}

			case '\t':
				switch(state) {
					case PR_FILE:
						string_new_from(&event.file_line, event_buffer.chars, event_buffer.count);
						state = PR_TYPE;
						break;

					case PR_INPUT: {
						n64 data;
						string_nterm(&event_buffer);

						errno = 0;
						data = strtol(event_buffer.chars, NULL, 10);
						assert(!errno);

						switch(event.type) {
							case MALLOC: event.as.malloc.size = data; break;
							case CALLOC: event.as.calloc.size = data; break;
							case REALLOC: event.as.realloc.size = data; break;
							case FREE:
							default: assert(0);
						}
						state = PR_OUTPUT;
						break;
					}

					default: assert(0);
				}

				string_clear(&event_buffer);
				break;

			case ' ':
				switch(state) {
					case PR_TYPE:
						if(string_equals(event_buffer, "MALLOC", strlen("MALLOC"))) {
							event.type = MALLOC;
						} else if(string_equals(event_buffer, "CALLOC", strlen("CALLOC"))) {
							event.type = CALLOC;
						} else if(string_equals(event_buffer, "REALLOC", strlen("REALLOC"))) {
							event.type = REALLOC;
						} else if(string_equals(event_buffer, "FREE", strlen("FREE"))) {
							event.type = FREE;
						} else {
							printf("Unrecognised pattern in pos %lu: "STR_FMT"\n", i, STR(event_buffer));
							exit(failure);
						}

						string_clear(&event_buffer);
						state = PR_INPUT;
						break;

					case PR_INPUT: {
						n64 data;
						string_nterm(&event_buffer);

						switch(event.type) {
							case CALLOC:
								errno = 0;
								event.as.calloc.memb = strtol(event_buffer.chars, NULL, 10);
								assert(!errno);
								break;

							case REALLOC:
								errno = 0;
								event.as.realloc.fptr.raw = strtol(event_buffer.chars, NULL, 16);
								assert(!errno);
								break;

							default: assert(0);
						}

						string_clear(&event_buffer);
						break;
					}

					default:
						printf("ERROR: Unexpected space (' ') in pos %lu\n", i);
						exit(failure);
				}
				break;

			default:
				string_append(&event_buffer, c);
		}
	}
}

void printdump(MEvents events) {
	n64 i;

	/* printf("%lu %x\n", events.count, (unsigned int)events.items[0].as.realloc.rptr); */
	/* printw((unsigned int)events.items[0].as.realloc.rptr); */
	/* printf("\n"); */
	for(i = 0; i < events.count; ++i) {
		MEvent event = events.items[i];
		Ptr ptr = {0};

		printf(STR_FMT, STR(event.file_line));

		switch(event.type) {
			case MALLOC:
				ptr = event.as.malloc.rptr;
				printf("\tMALLOC %d\t%x-%x-%x\n",
						event.as.malloc.size, ptr.offset, ptr.page, ptr.address);
				break;

			case CALLOC:
				ptr = event.as.calloc.rptr;
				printf("\tCALLOC %d %d\t%x-%x-%x\n",
						event.as.calloc.memb, event.as.calloc.size,
						ptr.offset, ptr.page, ptr.address);
				break;

			case REALLOC:
				ptr = event.as.realloc.fptr;
				printf("\tREALLOC %x-%x-%x %d\t", ptr.offset, ptr.page,
						ptr.address, event.as.realloc.size);

				ptr = event.as.realloc.rptr;
				printf("%x-%x-%x\n", ptr.offset, ptr.page, ptr.address);
				break;

			case FREE:
				ptr = event.as.free.ptr;
				printf("\tFREE %x-%x-%x\n", ptr.offset, ptr.page, ptr.address);
				break;

			default: assert(0);
		}
	}
}

char* shift(char*** argv, int* argc) {
	char* t;
	assert(*argc > 0);

	(*argc)--;
	t = (*argv)[*argc];
	(*argv)++;

	return t;
}

int inotify_setup() {
	int flags, events_fd = inotify_init();
	if(events_fd == -1) {
		printf("ERROR:%s:%d: Could not initialise file watcher: %s\n",
				__FILE__, __LINE__, strerror(errno));
		exit(failure);
	}

	/* Set fd to not be blocking */
	flags = fcntl(events_fd, F_GETFL, 0);
	fcntl(events_fd, F_SETFL, flags | O_NONBLOCK);

	return events_fd;
}

int inotify_watch(int events_fd, char* path) {
/* #define ALL_MASKS (IN_ACCESS|IN_ATTRIB|IN_CLOSE_WRITE|IN_CLOSE_NOWRITE|IN_CREATE|IN_DELETE|IN_DELETE_SELF|IN_MODIFY|IN_MOVE_SELF|IN_MOVED_FROM|IN_MOVED_TO|IN_OPEN) */
	int watch_d = inotify_add_watch(events_fd, path, IN_ACCESS);
	if(watch_d < 0) {
		printf("ERROR:%s:%d: Could not add watch to file '%s': %s\n",
				__FILE__, __LINE__, path, strerror(errno));
		exit(failure);
	}

	return watch_d;
}

void inotify_unwatch(int events_fd, int watch_d) {
	if(inotify_rm_watch(events_fd, watch_d) < 0) {
		printf("ERROR:%s:%d: Could not remove watch: %s\n",
				__FILE__, __LINE__, strerror(errno));
		exit(failure);
	}
}

bool inotify_pinged(int events_fd) {
	struct inotify_event event_buffer = {0};
	ssize_t size_read = 0;

	do {
		size_read = read(events_fd, &event_buffer, sizeof(struct inotify_event));
		if(size_read < 0) {
			if(errno == EAGAIN) {
				return false;
			}

			printf("ERROR:%s:%d: Could not read watch file: %s\n",
					__FILE__, __LINE__, strerror(errno));
			exit(failure);
		}

		assert(event_buffer.len == 0);

/* #define printmask(MASK) \ */
/* 		if(event_buffer.mask & MASK) printf(#MASK "\n") */
		/* printmask(IN_ACCESS); */
		/* printmask(IN_ATTRIB); */
		/* printmask(IN_CLOSE_WRITE); */
		/* printmask(IN_CLOSE_NOWRITE); */
		/* printmask(IN_CREATE); */
		/* printmask(IN_DELETE); */
		/* printmask(IN_DELETE_SELF); */
		/* printmask(IN_MODIFY); */
		/* printmask(IN_MOVE_SELF); */
		/* printmask(IN_MOVED_FROM); */
		/* printmask(IN_MOVED_TO); */
		/* printmask(IN_OPEN); */

		if(event_buffer.mask & IN_ATTRIB) {
			printf("\033[35mEvent emitted: \033[2mIN_ACCESS\033[0m");
			return true;
		}

	} while(size_read > 0);

	return false;
}


int main(int argc, char** argv) {
	HGL_State state = {0};
	n64 tick = 0;

	char* program = shift(&argv, &argc);
	char* dumppath;
	int events_fd, watch_d;

	if(argc < 1) {
		printf("Incorrect usage: %s <filepath>\n", program);
		exit(failure);
	}

	dumppath = shift(&argv, &argc);
	parsedump(dumppath, &state.events);

	if(HGL_load(DL_PATH)) exit(failure);

	SetTargetFPS(20);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello world MF!");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	events_fd = inotify_setup();
	watch_d = inotify_watch(events_fd, DL_PATH);

	HGL_init_fn(&state);
	while(!WindowShouldClose()) {
		if(IsKeyPressed(KEY_R) || inotify_pinged(events_fd)) {
			show_pending_screen();
			inotify_unwatch(events_fd, watch_d);

			if(HGL_reload())
				exit(failure);

			watch_d = inotify_watch(events_fd, DL_PATH);
			continue;
		}

		HGL_tick_fn(&state);
	}

	HGL_stop_fn(&state);
	HGL_unload();

	CloseWindow();
	return 0;
}
