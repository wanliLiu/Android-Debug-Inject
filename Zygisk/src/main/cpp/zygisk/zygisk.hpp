#pragma once

#include <jni.h>
#include <sys/types.h>


void hook_entry(void * handle);

void hookJniNativeMethods(JNIEnv *env, const char *clz, JNINativeMethod *methods, int numMethods);
