#include <core.h>
#include <stdlib.h>

/* #define DEBUG_MEMDEB_ENABLE 1 */

void *malloc_(size_t size, char* file, int line) {
	void* rptr = malloc(size);

#if DEBUG_MEMDEB_ENABLE
	printf("%s:%d\tMALLOC %lu\t", file, line, size);

	if(rptr == NULL) printf("0\n");
	else printf("%p\n", rptr);
#endif

	return rptr;
}

void *calloc_(size_t nmemb, size_t size, char* file, int line) {
	void* rptr = calloc(nmemb, size);

#if DEBUG_MEMDEB_ENABLE
	printf("%s:%d\tCALLOC %lu %lu\t", file, line, nmemb, size);
	if(rptr == NULL) printf("0\n");
	else printf("%p\n", rptr);
#endif

	return rptr;
}

void *realloc_(void* ptr, size_t size, char* file, int line) {
	void* rptr;

#if DEBUG_MEMDEB_ENABLE
	printf("%s:%d\tREALLOC ", file, line);
	if(ptr == NULL) printf("0");
	else printf("%p", ptr);
#endif

	rptr = realloc(ptr, size);

#if DEBUG_MEMDEB_ENABLE
	printf(" %lu\t", size);
	if(rptr == NULL) printf("0\n");
	else printf("%p\n", rptr);
#endif

	return rptr;
}

void free_(void* ptr, char* file, int line) {
#if DEBUG_MEMDEB_ENABLE
	printf("%s:%d\tFREE ", file, line);
	if(ptr == NULL) printf("0");
	else printf("%p", ptr);
	printf("\n");
#endif

	free(ptr);
}
