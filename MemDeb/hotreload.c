#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>

#include "hotdraw.h"

#define DL_PATH_LEN 50
char dl_path[DL_PATH_LEN];
void* dl_handle = NULL;

void (*HGL_init_fn)(HGL_State* state) = NULL;
void (*HGL_tick_fn)(HGL_State* state) = NULL;
void (*HGL_stop_fn)(HGL_State* state) = NULL;

error HGL_load(char* path) {
	assert(dl_handle == NULL && "Can't load the plugin without closing the old one!");

	strncpy(dl_path, path, (DL_PATH_LEN - 1));
	char* error = NULL;


	/* LOAD DLL */
	dl_handle = dlopen(dl_path, RTLD_NOW);
	if(dl_handle == NULL) {
		printf("\033[35mFailed to load dl\033[0m: %s\n", dlerror());
		return failure;
	}


	/* LOAD SYMBOLS */
	HGL_init_fn = dlsym(dl_handle, "init_fn");
	if((error = dlerror())) goto symbol_error;

	HGL_tick_fn = dlsym(dl_handle, "tick_fn");
	if((error = dlerror())) goto symbol_error;

	HGL_stop_fn = dlsym(dl_handle, "stop_fn");
	if((error = dlerror())) goto symbol_error;


	printf("\033[34mLoaded DL successfully\033[0m\n");
	return success;


symbol_error:
	printf("\033[35mFailed to load symbol setup\033[0m: %s\n", error);
	return failure;
}

error HGL_reload() {
	if(HGL_unload()) return failure;
	if(HGL_load(dl_path)) return failure;

	return success;
}

error HGL_unload() {
	HGL_init_fn = NULL;
	HGL_tick_fn = NULL;
	HGL_stop_fn = NULL;

	if(dlclose(dl_handle)) {
		printf("\033[35mFailed to unload DL\033[0m: %s\n", dlerror());
		return failure;
	}

	dl_handle = NULL;

	printf("\033[36mUnloaded DL Successfully\033[0m\n");
	return success;
}
