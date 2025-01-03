

#ifndef HOOKMANAGER_FAKE_DLFCN_H
#define HOOKMANAGER_FAKE_DLFCN_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
__attribute__ ((visibility ("default"))) void *fake_dlopen(const char *libpath, int flags);
__attribute__ ((visibility ("default"))) void fake_dlclose(void* handle);
__attribute__ ((visibility ("default"))) void *fake_dlsym(void *handle, const char *name);
#ifdef __cplusplus
};
#endif
#endif //HOOKMANAGER_FAKE_DLFCN_H
