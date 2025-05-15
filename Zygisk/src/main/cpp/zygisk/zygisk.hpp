#pragma once

#include <stdint.h>
#include <jni.h>
#include <vector>

extern void *self_handle;

void hook_functions() ;

void setValue(uintptr_t start_addr ,size_t size);

void revert_unmount_ksu();

void revert_unmount_magisk();

