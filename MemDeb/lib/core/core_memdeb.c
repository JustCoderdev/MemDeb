#include <core.h>
#include <stdlib.h>

void *malloc_deb_(size_t size, char* file, int line) {
	void* rptr = malloc(size);
	printf("%s:%d\tMALLOC %lu\t", file, line, size);

	if(rptr == NULL) printf("0\n");
	else printf("%p\n", rptr);

	return rptr;
}

void *calloc_deb_(size_t nmemb, size_t size, char* file, int line) {
	void* rptr = calloc(nmemb, size);
	printf("%s:%d\tCALLOC %lu %lu\t", file, line, nmemb, size);

	if(rptr == NULL) printf("0\n");
	else printf("%p\n", rptr);

	return rptr;
}

void *realloc_deb_(void* ptr, size_t size, char* file, int line) {
	void* rptr;
	printf("%s:%d\tREALLOC ", file, line);

	if(ptr == NULL) printf("0");
	else printf("%p", ptr);

	rptr = realloc(ptr, size);
	printf(" %lu\t", size);

	if(rptr == NULL) printf("0\n");
	else printf("%p\n", rptr);

	return rptr;
}

void free_deb_(void* ptr, char* file, int line) {
	printf("%s:%d\tFREE ", file, line);

	if(ptr == NULL) printf("0");
	else printf("%p", ptr);

	printf("\n");
	free(ptr);
}
