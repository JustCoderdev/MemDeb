/* JustCoderdev's Core library v4 (not semver)
 * */

#ifndef CORE_H_
#define CORE_H_

/* #define DEBUG_ENABLE 1 */
/* #define DEBUG_MEMDEB_ENABLE 1 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

/* Types */
typedef unsigned char uchar;

typedef unsigned char     n8;
typedef unsigned short    n16;
typedef unsigned int      n32;
typedef unsigned long int n64;

typedef signed char     i8;
typedef signed short    i16;
typedef signed int      i32;
typedef signed long int i64;

#define RL_BOOL_TYPE
typedef enum bool {
	true = (1 == 1),
	false = (1 != 1)
} bool;

typedef enum error {
	success = false,
	failure = true
} error;

typedef int Errno;

/* Strings */
typedef struct String {
	char* chars;
	n64 count;
	n64 capacity;
} String;

#define STR_FMT "%.*s"
#define STR(STRING) (int)(STRING).count, (STRING).chars

extern void string_new(String* string, n64 capacity);
extern void string_new_from(String* string, char* text, n64 text_len);
extern void string_clear(String* string);
extern void string_from(String* string, char* text, n64 text_len);
extern void string_cpy(String* string, char* text, n64 text_len);
extern void string_append(String* string, char chr);
extern void string_remove(String* string, n64 count);
extern bool string_equals(String strA, char* strB, n64 strB_len);
extern void string_free(String* string);


/* Memory debug */
#if DEBUG_MEMDEB_ENABLE
#	define dmalloc(SIZE) malloc_deb_(SIZE, __FILE__, __LINE__)
#	define dfree(PTR)    free_deb_(PTR, __FILE__, __LINE__)
#	define dcalloc(NMEMB, SIZE) calloc_deb_(NMEMB, SIZE, __FILE__, __LINE__)
#	define drealloc(PTR, SIZE)  realloc_deb_(PTR, SIZE, __FILE__, __LINE__)
#else
#	define dmalloc(SIZE) malloc(SIZE)
#	define dfree(PTR)    free(PTR)
#	define dcalloc(NMEMB, SIZE) calloc(NMEMB, SIZE)
#	define drealloc(PTR, SIZE)  realloc(PTR, SIZE)
#endif

extern void *malloc_deb_(size_t size, char* file, int line);
extern void free_deb_(void* ptr, char* file, int line);
extern void *calloc_deb_(size_t nmemb, size_t size, char* file, int line);
extern void *realloc_deb_(void* ptr, size_t size, char* file, int line);


/* Macros */
#ifndef min
#define min(A, B) ((A) > (B) ? (B) : (A))
#endif

#ifndef max
#define max(A, B) ((A) > (B) ? (A) : (B))
#endif


/* Bit level stuff */
extern void printb(n8 byte);
extern void printw(n32 word);

extern void savebuff(FILE *file, char *buffer, n64 buff_len);


/* Net */
/* typedef union ip4_addr { */
	/* struct in_addr addr; */
	/* struct { n8 D, C, B, A; }; */
	/* n8 bytes_rev[4]; */
/* } in_addr; */

/* Convert decimal ipv4 address (192.168.42.069) to `struct in_addr` */
extern struct in_addr addr_to_bytes(n8 A, n8 B, n8 C, n8 D);

/* Given the hostname the function will try to resolve it and fill the
 * `address` field */
extern error hostname_resolve(const char *hostname, struct in_addr *address);


/* Buffer */
/* Return the index of the chr or -1 */
extern i64 buffer_find_chr(char chr, char *buffer, n64 len);

/* Return the index of the first char that is not a delimiter or -1 */
extern i64 buffer_skip_str(char *del, n64 del_len, char *buffer, n64 buff_len);

/* Copy from src to dest buffers until the delimiter is reached */
extern n64 buffer_copy_until_chr(char delimiter,
            char *src_buffer, n64 src_len,
            char *dest_buffer, n64 dest_len);

/* Copy from src to dest buffers until the delimiter is reached */
extern n64 buffer_copy_until_str(char *delimiter, n64 del_len,
            char *src_buffer, n64 src_len,
            char *dest_buffer, n64 dest_len);


/* Dinamic array fast implementation --------------------------- */

/* typedef struct Array { */
/* 	void* items; */
/* 	n64 count; */
/* 	n64 capacity; */
/* } Array; */

#define arr_new(arr, cap) \
	do { \
		if((arr)->capacity < (cap)) { \
			(arr)->items = drealloc((arr)->items, (cap) * sizeof(*(arr)->items)); \
			if((arr)->items == NULL) { \
				printf("ERROR:%s:%d: Couldn't resize array, message: %s", \
						__FILE__, __LINE__, strerror(errno)); \
				exit(failure); \
			} \
			(arr)->capacity = (cap); \
		} \
		(arr)->count = 0; \
	} while(0)

#define arr_from(arr, src, src_len) do { \
	if((arr)->capacity < (src_len)) { \
		(arr)->items = drealloc((arr)->items, (src_len) * sizeof(*(arr)->items)); \
		if((arr)->items == NULL) { \
			printf("ERROR:%s:%d: Couldn't resize array, message: %s", \
					__FILE__, __LINE__, strerror(errno)); \
			exit(failure); \
		} \
		(arr)->capacity = (src_len); \
	} \
	(void)memcpy((arr)->items, (src)->items, (src_len)); \
	(arr)->count = (src_len); \
} while(0)

#define arr_append(arr, item) do { \
	if((arr)->capacity < (arr)->count + 1) { \
		(arr)->items = drealloc((arr)->items, ((arr)->count + 1) * 2 * sizeof(*(arr)->items)); \
		if((arr)->items == NULL) { \
			printf("ERROR:%s:%d: Couldn't resize array, message: %s", \
					__FILE__, __LINE__, strerror(errno)); \
			exit(failure); \
		} \
		(arr)->capacity = ((arr)->count + 1) * 2; \
	} \
	(arr)->items[(arr)->count] = (item); \
	(arr)->count++; \
} while(0)
/* Dinamic array fast implementation END ----------------------- */

#endif /* CORE_ */

