#include "hotdraw.h"
#include <core.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <linux/time.h>
int nanosleep(const struct timespec *req, struct timespec *rem);

#include <raylib.h>


#define DL_PATH ("libhotfile.so")


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
const n16 WINDOW_HEIGHT = 500;

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
			case '\n':
				string_append(&buffer, '\0');
				switch(event.type) {
					case MALLOC:
						event.as.malloc.rptr = atol(buffer.chars);
						break;

					case CALLOC:
						event.as.calloc.rptr = atol(buffer.chars);
						break;

					case REALLOC:
						event.as.realloc.rptr = atol(buffer.chars);
						break;

					case FREE:
						event.as.free.ptr = atol(buffer.chars);
						break;

					default: assert(0);
				}
				arr_append(events, event);
				string_clear(&buffer);
				state = PR_FILE;
				break;

			case '\t':
				switch(state) {
					case PR_FILE:
						string_new_from(&event.file_line, buffer.chars, buffer.count);
						state = PR_TYPE;
						break;

					case PR_INPUT:
						string_append(&buffer, '\0');
						switch(event.type) {
							case MALLOC:
								event.as.malloc.size = atol(buffer.chars);
								break;

							case CALLOC:
								event.as.calloc.size = atol(buffer.chars);
								break;

							case REALLOC:
								event.as.realloc.new_size = atol(buffer.chars);
								break;

							case FREE:
							default: assert(0);
						}
						state = PR_OUTPUT;
						break;

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

					case PR_INPUT:
						switch(event.type) {
							case CALLOC:
								string_append(&buffer, '\0');
								event.as.calloc.memb = atol(buffer.chars);
								break;

							case REALLOC:
								string_append(&buffer, '\0');
								event.as.realloc.fptr = atol(buffer.chars);
								break;

							default: assert(0);
						}
						string_clear(&buffer);
						break;

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

	printf("%lu\n", events.count);
	for(i = 0; i < events.count; ++i) {
		MEvent event = events.items[i];

		printf(STR_FMT, STR(event.file_line));

		switch(event.type) {
			case MALLOC:
				printf("\tMALLOC %lu\t%x\n",
						event.as.malloc.size, (int)event.as.malloc.rptr);
				break;

			case CALLOC:
				printf("\tCALLOC %lu %lu\t%x\n",
						event.as.calloc.memb, event.as.calloc.size,
						(int)event.as.calloc.rptr);
				break;

			case REALLOC:
				printf("\tREALLOC %x %lu\t%x\n", (int)event.as.realloc.fptr,
						event.as.realloc.new_size, (int)event.as.realloc.rptr);
				break;

			case FREE:
				printf("\tFREE %x\n", (int)event.as.free.ptr);
				break;

			default: assert(0);
		}
	}

}

int main(void) {
	HGL_State state = {0};
	n64 tick = 0;

	parsedump("memdump", &state.events);
	printdump(state.events);
	exit(success);

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
