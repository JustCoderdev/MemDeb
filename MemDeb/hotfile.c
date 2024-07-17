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

#define BAR_WIDTH (DPAD)
#define EVENT_SIDEBAR_WIDTH (300 + BAR_WIDTH + DPAD)

const char* EVENT_TYPE_LABEL[4] = { "MALLOC", "CALLOC", "REALLOC", "FREE" };

Color htoc_(n32 hex) { /* 0x RR GG BB AA */
	Color color = {0};
	color.r = hex >> 24;
	color.g = hex >> 16;
	color.b = hex >> 8;
	color.a = hex;
	return color;
}

bool CollisionPointRec(Vector2 p, Rectangle r) {
	return p.x > r.x && p.x < r.x + r.width
		&& p.y > r.y && p.y < r.y + r.height;
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
	state->cevent_index = 0;
	state->offset = 0;
}


void draw_bar(HGL_State* state, Rectangle bbox, n64 progress, n64 total, n64 visible, Color color) {
	Rectangle cursor = {0};
	float seg = bbox.height / total;

	cursor.height = seg * visible;

	{
		/* Handle input */
		Vector2 mouse = GetMousePosition();
		bool hovering = CollisionPointRec(mouse, bbox);
		bool dragging = hovering && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

		SetMouseCursor(dragging ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
		if(dragging) {
			float ccenter = cursor.height / 2;
			float mouseprog = clamp(bbox.y, mouse.y - ccenter, bbox.y + bbox.height - cursor.height);
			progress = max(0, mouseprog - ccenter) / seg;
			state->offset = progress;
		}
	}

	cursor.width = bbox.width;
	cursor.y = bbox.y + (seg * progress);
	cursor.x = bbox.x;

	DrawRectangleLinesEx(bbox, BORD, color);
	DrawRectangleRec(cursor, color);

}


void tick_fn(HGL_State* state) {
	String* buffer = &state->buffer;
	Vector2 mouse = GetMousePosition();

	const n32 WINDOW_WIDTH = GetScreenWidth();
	const n32 WINDOW_HEIGHT = GetScreenHeight();

	n8 events_shown;
	Rectangle event_rect = {0};
	event_rect.x = WINDOW_WIDTH - EVENT_SIDEBAR_WIDTH + PAD;
	event_rect.width = EVENT_SIDEBAR_WIDTH  - BAR_WIDTH - DPAD - HPAD;
	event_rect.height = FONT_SIZE * 2 + PAD;
	events_shown = (WINDOW_HEIGHT - DPAD) / (event_rect.height + PAD);

	BeginDrawing();
	ClearBackground(CLR_BACKG);

#if DEBUG_ENABLE
	ntos(fps_string, FPS_STR_LEN, (n64)GetFPS());
	DrawText(fps_string, PAD, PAD, FONT_SIZE, CLR_DEBUG);

	ntos(tick_string, TICK_STR_LEN, state->tick);
	{
		n8 text_width = MeasureText("2", FONT_SIZE) * strlen(tick_string);
		DrawText(tick_string, WINDOW_WIDTH - EVENT_SIDEBAR_WIDTH - DPAD - text_width,
				PAD, FONT_SIZE, CLR_DEBUG);
		state->tick++;
	}
#endif

	/* Handle input */
	if(IsKeyPressed(KEY_DOWN)) {
		state->cevent_index++;
		if(state->cevent_index >= state->events.count)
			state->cevent_index = 0;

		state->offset = clamp(0,
				(state->cevent_index / events_shown) * events_shown,
				state->events.count - events_shown);
	}

	if(IsKeyPressed(KEY_UP)) {
		state->cevent_index--;
		if(state->cevent_index > state->events.count)
			state->cevent_index = state->events.count - 1;

		state->offset = clamp(0,
				(state->cevent_index / events_shown) * events_shown,
				state->events.count - events_shown);
	}


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
		n16 i;
		for(i = 0; i < events_shown + 1; ++i) {
			n32 event_id = (i + state->offset) % state->events.count;
			MEvent event = state->events.items[event_id];

			Color color = CLR_FOREG;
			bool hovering;

			event_rect.y = PAD + (event_rect.height + PAD) * i;
			hovering = CollisionPointRec(mouse, event_rect);

			/* Handle inputs */
			if(hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				state->cevent_index = event_id;
			}

			/* Draw background */
			if(event_id <= state->cevent_index) {
				DrawRectangleRec(event_rect, color);

				if(hovering) color = RED;
				DrawRectangleLinesEx(event_rect, HBORD, color);

				color = CLR_BACKG;
			} else {
				if(hovering) color = RED;
				DrawRectangleLinesEx(event_rect, HBORD, color);

				color = CLR_FOREG;
			}

			string_clear(buffer);

			switch(event.type) {
				case MALLOC:
					/* #1 MALLOC 64B */
					buffer->count = 1 + snprintf(buffer->chars,
							buffer->capacity, "#%u %s %dB", event_id,
							EVENT_TYPE_LABEL[event.type], event.as.malloc.size);
					break;
					/* 0xABCDEF01 */

				case REALLOC:
					/* #1 REALLOC 0xABC 64B */
					buffer->count = 1 + snprintf(buffer->chars,
							buffer->capacity, "#%u %s 0x%x %dB", event_id,
							EVENT_TYPE_LABEL[event.type], event.as.realloc.fptr,
							event.as.realloc.new_size);
					break;
					/* 0xABCDEF012345678 */

				case CALLOC:
					/* #1 CALLOC 1 64b */
					buffer->count = 1 + snprintf(buffer->chars,
							buffer->capacity, "#%u %s %d %dB", event_id,
							EVENT_TYPE_LABEL[event.type], event.as.calloc.memb,
							event.as.calloc.size);
					break;
					/* 0xABCDEF0123 */

				case FREE:
					/* #1 FREE 0xABC */
					buffer->count = 1 + snprintf(buffer->chars,
							buffer->capacity, "#%u %s 0x%x", event_id,
							EVENT_TYPE_LABEL[event.type], event.as.free.ptr);
					break;

				default:
					printf("event.type = %d\n", event.type);
					assert(0);
			}

			DrawText(buffer->chars, event_rect.x + HPAD, event_rect.y + HPAD, FONT_SIZE, color);

 			/* hovering */
			if(hovering) {
				DrawText(event.file_line.chars, event_rect.x + HPAD, event_rect.y + FONT_SIZE + HPAD, FONT_SIZE, color);
			} else {
				string_clear(buffer);

				switch(event.type) {
					case MALLOC:
						/* 0xABCDEF01 */
						buffer->count = 1 + snprintf(buffer->chars,
								buffer->capacity, "0x%x",
								event.as.malloc.rptr);
						break;

					case REALLOC:
						/* 0xABCDEF012345678 */
						buffer->count = 1 + snprintf(buffer->chars,
								buffer->capacity, "0x%x",
								event.as.realloc.rptr);
						break;

					case CALLOC:
						/* 0xABCDEF0123 */
						buffer->count = 1 + snprintf(buffer->chars,
								buffer->capacity, "0x%x",
								event.as.calloc.rptr);
						break;

					case FREE: break;
					default: assert(0);
				}

				DrawText(buffer->chars, event_rect.x + HPAD, event_rect.y + FONT_SIZE + HPAD, FONT_SIZE, color);
			}
		}
	}

	{
		Rectangle rect = {0};
		bool hover_queue;

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

		hover_queue = CollisionPointRec(mouse, rect);
		DrawRectangleLinesEx(rect, BORD, CLR_FOREG);
		if(hover_queue) {
			i8 off = (i8)GetMouseWheelMove();
			assert(off == -1 || off == 0 || off == 1);

			state->offset -= off;
			if(off == -1) { /* Down v */
				if(state->offset + events_shown >= state->events.count)
					state->offset = state->events.count - events_shown - 1;
			}

			if(off == 1) { /* Up ^ */
				/* IF offset overflows... */
				if(state->offset >= state->events.count)
					state->offset = 0;
			}
		}

		/* Scroll bar */
		rect.x = WINDOW_WIDTH - BAR_WIDTH - PAD;
		rect.y = PAD;
		rect.width = BAR_WIDTH;
		rect.height = WINDOW_HEIGHT - DPAD;
		draw_bar(state, rect, state->offset, state->events.count,
				events_shown, CLR_FOREG);

		/* Draw mouse position */
		DrawCircleV(mouse, 5, RED);
	}

	EndDrawing();
}

void stop_fn(HGL_State* state) {
	(void)state;
}

