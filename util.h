
/*
 * 
 * Copyright (c) 2015. Lukas Dresel, Julius Lohmann, Benedikt Lorch, Burkhard Ringlein, Andreas Rupp, Yuriy Sulima, Carola Touchy
 * 
 * This file is part of GarbageCollector. 
 * 
 *	 GarbageCollector is free software: you can redistribute it and/or modify
 * 	 it under the terms of the GNU General Public License as published by
 * 	 the Free Software Foundation, either version 3 of the License, or
 * 	 (at your option) any later version.
 * 
 * 	 GarbageCollector is distributed in the hope that it will be useful,
 * 	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	 GNU General Public License for more details.
 * 
 * 	 You should have received a copy of the GNU General Public License
 * 	 along with GarbageCollector.  If not, see <http://www.gnu.org/licenses/>.
 *
 */ 




#pragma once 

#include <sys/stat.h>
#include <iostream>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
#define LOG_DEBUG(...) do {printf("[DEBUG]: " __VA_ARGS__);} while(0);
#else
#define LOG_DEBUG(...) do {} while(0);
#endif // DEBUG

#define PRINT_METHOD_NAME

#ifdef PRINT_METHOD_NAME
#define DEBUGINFO(module, method) "[" #module "::" #method "] "
#else
#define DEBUGINFO(module, method) "[" #module "] "
#endif

size_t getFilesize(const char* filename);
uint64_t getFileOrDeviceSize(const char* realPath, int fd);
bool fread_check_error(const char* descriptor, unsigned char* buf, size_t numBytes, FILE* file);
