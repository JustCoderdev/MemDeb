# Memory Debugger

---

"MemDeb" is a simple GUI application written in C89 (and Raylib help, thx)
that shows on screen a timeline of allocations and freeings and let's you
jump in between and analize what was deallocated, where, what happened and
all the usual segfault shenanigans (like double freeing a pointer...)

## The log format

> Note: null pointers should be represented by 0 and not (nil)!
>   All rows must start with the filename followed by a colon (':')
>   and the line of the function terminated by a tab ('\t'), what follows is
>   specific to the type of allocation/deallocation happening

> Full Examples (\t and \n must be replaced with the actual whitespace)
>   `jlexer.c:19\tREALLOC 0x17d0490 880\t0x17d16d0\n`
>   `lib/core/core_str.c:33\tMALLOC 9\t0x17d1a50\n`
>   `lib/core/core_str.c:109\tFREE 0x17d0630\n`

To know stuff memdeb reads and parses a file called `memdump` that needs to
have rows of this specific format (every row is separated by a newline '\n')

`file:line\t`

Example: `core/core_str.c:86\t`

### MALLOC

TAG SIZE RPTR
Example: `MALLOC 4\t0x0`

### CALLOC

TAG MEMB SIZE RPTR
Example: `CALLOC 18 2\t0x0`

### REALLOC

TAG FPTR SIZE RPTR
Example: `REALLOC 0x0 10\t0x0`

### FREE

TAG PTR
Example: `FREE\t0x0`


