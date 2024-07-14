#include <hotdraw.h>
#include <core.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
extern int snprintf(char* str, size_t size, const char* format, ...);

#include <raylib.h>


#define HPAD 5
#define PAD 10
#define DPAD 20

#define FONT_SIZE 20

#define HBORD 2
#define BORD 4
#define DBORD 8

const char* EVENT_TYPE_LABEL[4] = { "MALLOC", "CALLOC", "REALLOC", "FREE" };

Color htoc_(n32 hex) { /* 0x RR GG BB AA */
	Color color = {0};
	color.r = hex >> 24;
	color.g = hex >> 16;
	color.b = hex >> 8;
	color.a = hex;
	return color;
}

Color CLR_FOREG = {  24, 195, 124, 255 };
Color CLR_DEBUG = {  62,  62,  62, 255 };
Color CLR_BACKG = {  30,  30,  30, 255 };


#define TICK_STR_LEN (20 + 1)
#define FPS_STR_LEN (2 + 1)

char tick_string[TICK_STR_LEN] = {0};
char fps_string[FPS_STR_LEN] = {0};
void ntos(char* string, const n64 len, n64 number) {
	n64 i;

	for(i = 0; i < len - 1; ++i) {
		if(number == 0) {
			string[len - i - 2] = '-';
			continue;
		}

		{
			n8 rest = number % 10;
			string[len - i - 2] = '0' + rest;
			number -= rest;
		}

		if(number != 0) number /= 10;
	}

	string[len - 1] = '\0';
}

void init_fn(HGL_State* state) {
	printf("Loaded module!\n");
	state->tick = 0;
	string_new(&state->buffer, 64);
}

void tick_fn(HGL_State* state) {
	String* buffer = &state->buffer;
	Vector2 mouse = GetMousePosition();
	const n32 WINDOW_WIDTH = GetScreenWidth();
	const n32 WINDOW_HEIGHT = GetScreenHeight();

	const n32 EVENT_SIDEBAR_WIDTH = WINDOW_WIDTH / 4;

	BeginDrawing();
	ClearBackground(CLR_BACKG);

#if DEBUG_ENABLE
	ntos(fps_string, FPS_STR_LEN, (n64)GetFPS());
	DrawText(fps_string, PAD, PAD, FONT_SIZE, CLR_DEBUG);

	{
		ntos(tick_string, TICK_STR_LEN, state->tick);
		n8 text_width = MeasureText("2", FONT_SIZE) * strlen(tick_string);
		DrawText(tick_string, WINDOW_WIDTH - PAD - text_width,
				PAD, FONT_SIZE, CLR_DEBUG);
		state->tick++;
	}
#endif

	/* Design
	 *
	 * Memory layout  Event que
	 * +-+-+-+-+-+-+-+-+-+
	 * |             |   |
	 * |             |   |
	 * |             |   |
	 * |             |   |
	 * +-+-+-+-+-+-+-+-+-+
	 * */

	{
		n64 i;
		n64 selected_id = 0;

		for(i = 0; i < state->events.count; ++i) {
			Color color = CLR_FOREG;
			MEvent event = state->events.items[i];

			Rectangle rect = {0};
			rect.x = WINDOW_WIDTH - EVENT_SIDEBAR_WIDTH + PAD;
			rect.width = EVENT_SIDEBAR_WIDTH - DPAD;
			rect.height = FONT_SIZE * 2 + PAD;
			rect.y = 4 * PAD + (rect.height + PAD) * i;

			if(selected_id == i) {
				DrawRectangleRec(rect, color);
				color = CLR_BACKG;
			} else {
				DrawRectangleLinesEx(rect, HBORD, color);
			}

			string_clear(buffer);

			switch(event.type) {
				case MALLOC:
					/* #1 MALLOC 64B */
					buffer->count = 1 + snprintf(buffer->chars, buffer->capacity, "#%lu %s %dB", i,
							EVENT_TYPE_LABEL[event.type], event.as.malloc.size);
					break;
					/* 0xABCDEF01 */

				case REALLOC:
					/* #1 REALLOC 0xABC 64B */
					buffer->count = 1 + snprintf(buffer->chars, buffer->capacity, "#%lu %s 0x%x %dB",
							i, EVENT_TYPE_LABEL[event.type], event.as.realloc.fptr,
							event.as.realloc.new_size);
					break;
					/* 0xABCDEF012345678 */

				case CALLOC:
					/* #1 CALLOC 1 64b */
					buffer->count = 1 + snprintf(buffer->chars, buffer->capacity, "#%lu %s %d %dB",
							i, EVENT_TYPE_LABEL[event.type], event.as.calloc.memb,
							event.as.calloc.size);
					break;
					/* 0xABCDEF0123 */

				case FREE:
					/* #1 FREE 0xABC */
					buffer->count = 1 + snprintf(buffer->chars, buffer->capacity, "#%lu %s 0x%x",
							i, EVENT_TYPE_LABEL[event.type], event.as.free.ptr);
					break;

				default: assert(0);
			}

			DrawText(buffer->chars, rect.x + HPAD, rect.y + HPAD, FONT_SIZE, RED);

 			/* hovering */
			if(mouse.x > rect.x && mouse.x < rect.x + rect.width &&
				mouse.y > rect.y && mouse.y < rect.y + rect.height) {
				DrawText(event.file_line.chars, rect.x + HPAD, rect.y + FONT_SIZE + HPAD, FONT_SIZE, color);
			} else {
				string_clear(buffer);

				switch(event.type) {
					case MALLOC:
						/* 0xABCDEF01 */
						buffer->count = 1 + snprintf(buffer->chars,
								buffer->capacity, "0x%x", event.as.malloc.rptr);
						break;

					case REALLOC:
						/* 0xABCDEF012345678 */
						buffer->count = 1 + snprintf(buffer->chars,
								buffer->capacity, "0x%x", event.as.realloc.rptr);
						break;

					case CALLOC:
						/* 0xABCDEF0123 */
						buffer->count = 1 + snprintf(buffer->chars,
								buffer->capacity, "0x%x",
								event.as.calloc.rptr);
						break;

					case FREE:
						/* #1 FREE 0xABC */
						buffer->count = 1 + snprintf(buffer->chars, buffer->capacity, "#%lu %s 0x%x",
								i, EVENT_TYPE_LABEL[event.type], event.as.free.ptr);
						break;

					default: assert(0);
				}

				DrawText(buffer->chars, rect.x + HPAD, rect.y + FONT_SIZE + HPAD, FONT_SIZE, color);

			}

		}
	}

	{
		Rectangle rect = {0};

		/* Extern border */
		rect.x = 0;
		rect.y = 0;
		rect.width = WINDOW_WIDTH;
		rect.height = WINDOW_HEIGHT;
		DrawRectangleLinesEx(rect, BORD, CLR_FOREG);

		/* Event queue border */
		rect.x = WINDOW_WIDTH - EVENT_SIDEBAR_WIDTH;
		rect.y = 0;
		rect.width = EVENT_SIDEBAR_WIDTH;
		rect.height = WINDOW_HEIGHT;
		DrawRectangleLinesEx(rect, BORD, CLR_FOREG);

		DrawCircleV(mouse, 5, RED);
	}

	EndDrawing();
}

void stop_fn(HGL_State* state) {
	(void)state;
}

