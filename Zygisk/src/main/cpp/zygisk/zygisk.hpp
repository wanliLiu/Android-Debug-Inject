#pragma once

#include <stdint.h>
#include <jni.h>
#include <vector>

extern void *self_handle;

void hook_functions(void* so_start_addr,   size_t so_size) ;

void revert_unmount_ksu();

void revert_unmount_magisk();

