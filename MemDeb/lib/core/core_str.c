/* String module for JustCoderdev Core library v1
 * */

#include <core.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/* #define DEBUG_STRING_ENABLE 1 */

void string_new(String* string, n64 capacity) {
	string->chars = dmalloc(capacity * sizeof(char));
	if(string->chars == NULL) {
		printf("ERROR:%s:%d: Couldn't mallocate string, error: %s",
				__FILE__, __LINE__, strerror(errno));
		exit(failure);
	}

#if DEBUG_STRING_ENABLE
	printf("DEBUG: Mallocated string at %p with size %lu\n", string->chars, capacity * sizeof(char));
#endif

	string->capacity = capacity;

	string->count = 0;
}

void string_new_from(String* string, char* text, n64 text_len) {
	string->chars = dmalloc(text_len * sizeof(char));
	if(string->chars == NULL) {
		printf("ERROR:%s:%d: Couldn't mallocate string, error: %s",
				__FILE__, __LINE__, strerror(errno));
		exit(failure);
	}

#if DEBUG_STRING_ENABLE
	printf("DEBUG: Mallocated string at %p with size %lu\n", string->chars, text_len * sizeof(char));
#endif

	string->capacity = text_len;

	strncpy(string->chars, text, text_len);
	string->count = text_len;
}

void string_clear(String* string) {
	string->count = 0;
}

void string_from(String* string, char* text, n64 text_len) {
	if(string->capacity < text_len) {
		string->chars = drealloc(string->chars, text_len * sizeof(char));
		if(string->chars == NULL) {
			printf("ERROR:%s:%d: Couldn't reallocate string, error: %s",
					__FILE__, __LINE__, strerror(errno));
			exit(failure);
		}
		string->capacity = text_len;
	}

	strncpy(string->chars, text, text_len);
	string->count = text_len;
}

void string_append(String* string, char chr) {
	if(string->capacity < string->count + 1) {
		string->chars = drealloc(string->chars, (string->count + 1) * 2 * sizeof(char));
		if(string->chars == NULL) {
			printf("ERROR:%s:%d: Couldn't resize string, error: %s",
					__FILE__, __LINE__, strerror(errno));
			exit(failure);
		}
		string->capacity = (string->count + 1) * 2;
	}

	string->chars[string->count] = chr;
	string->count++;
}

void string_remove(String* string, n64 count) {
	string->count -= (count > string->count)
		? string->count
		: count;
}

bool string_equals(String strA, char* strB, n64 strB_len) {
	n64 i;

	if(strA.count != strB_len) return false;

	for(i = 0; i < strB_len; ++i)
		if(strA.chars[i] != strB[i])
			return false;

	return true;
}

void string_free(String* string) {
	if(string->chars != NULL) {

#if DEBUG_STRING_ENABLE
		printf("DEBUG: Freeing string at %p ('"STR_FMT"')\n", string->chars, STR(*string));
#endif

		dfree(string->chars);
	}

	string->count = 0;
	string->capacity = 0;
}
