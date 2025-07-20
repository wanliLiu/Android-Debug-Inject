//
// Created by rzx on 2025/7/20.
//

#ifndef ANDROID_DEBUG_INJECT_MODULE_H
#define ANDROID_DEBUG_INJECT_MODULE_H

#include "string"

void list_modules(std::string );
void add_enable_module( char* module_path,char* packageName);
#endif //ANDROID_DEBUG_INJECT_MODULE_H
