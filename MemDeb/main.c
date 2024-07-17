#include "hotdraw.h"
#include <core.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <linux/time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);

#include <raylib.h>


#define DL_PATH ("libhotfile.so")

typedef union Ptr {
	n32 raw;
	struct { /* Reversed order */
		unsigned address: 12;
		unsigned page: 4;
		unsigned offset: 16;
	};
} Ptr;

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
	String buffer = {0};
	MEvent event = {0};
	enum { PR_FILE, PR_TYPE, PR_INPUT, PR_OUTPUT } state = PR_FILE;
	char c;

	FILE* file = fopen(filepath, "r");
	assert(file);

	while((c = getc(file)) != EOF) {
		switch(c) {
			case '\n': {
				n64 data;

				string_nterm(&buffer);

				errno = 0;
				data = strtol(buffer.chars, NULL, 16);
 				assert(!errno);

				switch(event.type) {
					case MALLOC: event.as.malloc.rptr = data; break;
					case CALLOC: event.as.calloc.rptr = data; break;
					case REALLOC: event.as.realloc.rptr = data; break;
					case FREE: event.as.free.ptr = data; break;
					default: assert(0);
				}
				arr_append(events, event);
				string_clear(&buffer);
				state = PR_FILE;
				break;
			}

			case '\t':
				switch(state) {
					case PR_FILE:
						string_new_from(&event.file_line, buffer.chars, buffer.count);
						state = PR_TYPE;
						break;

					case PR_INPUT: {
						n64 data;
						string_nterm(&buffer);

						errno = 0;
						data = strtol(buffer.chars, NULL, 10);
						assert(!errno);

						switch(event.type) {
							case MALLOC: event.as.malloc.size = data; break;
							case CALLOC: event.as.calloc.size = data; break;
							case REALLOC: event.as.realloc.new_size = data; break;
							case FREE:
							default: assert(0);
						}
						state = PR_OUTPUT;
						break;
					}

					default: assert(0);
				}

				string_clear(&buffer);
				break;

			case ' ':
				switch(state) {
					case PR_TYPE:
						if(string_equals(buffer, "MALLOC", strlen("MALLOC"))) {
							event.type = MALLOC;
						} else if(string_equals(buffer, "CALLOC", strlen("CALLOC"))) {
							event.type = CALLOC;
						} else if(string_equals(buffer, "REALLOC", strlen("REALLOC"))) {
							event.type = REALLOC;
						} else if(string_equals(buffer, "FREE", strlen("FREE"))) {
							event.type = FREE;
						} else {
							printf(STR_FMT"\n", STR(buffer));
							assert(0);
						}

						string_clear(&buffer);
						state = PR_INPUT;
						break;

					case PR_INPUT: {
						n64 data;
						string_nterm(&buffer);

						switch(event.type) {
							case CALLOC:
								errno = 0;
								event.as.calloc.memb = strtol(buffer.chars, NULL, 10);
								assert(!errno);
								break;

							case REALLOC:
								errno = 0;
								event.as.realloc.fptr = strtol(buffer.chars, NULL, 16);
								assert(!errno);
								break;

							default: assert(0);
						}

						string_clear(&buffer);
						break;
					}

					default: assert(0);
				}
				break;

			default:
				string_append(&buffer, c);
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
				ptr.raw = event.as.malloc.rptr;
				printf("\tMALLOC %d\t%x-%x-%x\n",
						event.as.malloc.size, ptr.offset, ptr.page, ptr.address);
				break;

			case CALLOC:
				ptr.raw = event.as.calloc.rptr;
				printf("\tCALLOC %d %d\t%x-%x-%x\n",
						event.as.calloc.memb, event.as.calloc.size,
						ptr.offset, ptr.page, ptr.address);
				break;

			case REALLOC:
				ptr.raw = event.as.realloc.fptr;
				printf("\tREALLOC %x-%x-%x %d\t", ptr.offset, ptr.page,
						ptr.address, event.as.realloc.new_size);

				ptr.raw = event.as.realloc.rptr;
				printf("%x-%x-%x\n", ptr.offset, ptr.page, ptr.address);
				break;

			case FREE:
				ptr.raw = event.as.free.ptr;
				printf("\tFREE %x-%x-%x\n", ptr.offset, ptr.page, ptr.address);
				break;

			default: assert(0);
		}
	}

}

int main(void) {
	HGL_State state = {0};
	n64 tick = 0;

	parsedump("memdump", &state.events);
	/* printdump(state.events); */

	if(HGL_load(DL_PATH)) exit(failure);

	SetTargetFPS(20);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello world MF!");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	HGL_init_fn(&state);
	while(!WindowShouldClose()) {
		if(IsKeyPressed(KEY_R)) {
			show_pending_screen();

			if(HGL_reload())
				exit(failure);

			continue;
		}

		HGL_tick_fn(&state);
	}

	HGL_stop_fn(&state);
	HGL_unload();

	CloseWindow();
	return 0;
}
