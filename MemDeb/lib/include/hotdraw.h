#ifndef HGL_HOTDRAW_
#define HGL_HOTDRAW_

#include <core.h>

typedef enum MEvent_Type { MALLOC = 0, CALLOC, REALLOC, FREE } MEvent_Type;
typedef struct MEvent {
	String file_line;
	MEvent_Type type;

	union {
		struct {
			/* "%s:%d\tMALLOC %luB\t%p\n" */
			n32 size;
			n32 rptr;
		} malloc;

		struct {
			/* "%s:%d\tCALLOC %lu %luB\t%p\n" */
			n32 memb;
			n32 size;
			n32 rptr;
		} calloc;

		struct {
			/* "%s:%d\tREALLOC %p %luB\t%p\n" */
			n32 fptr;
			n32 new_size;
			n32 rptr;
		} realloc;

		struct {
			/* "%s:%d\tFREE %p\n" */
			n32 ptr;
		} free;
	} as;
} MEvent;
typedef struct MEvents {
	MEvent* items;
	n64 count;
	n64 capacity;
} MEvents;

typedef struct {
	MEvents events;
	String buffer;
	n64 tick;
	i64 cevent_index;
	i64 offset;
} HGL_State;

extern error HGL_load(char* path);
extern error HGL_reload(void);
extern error HGL_unload(void);

extern void (*HGL_init_fn)(HGL_State* state);
extern void (*HGL_tick_fn)(HGL_State* state);
extern void (*HGL_stop_fn)(HGL_State* state);

#endif /* HGL_HOTDRAW_ */
