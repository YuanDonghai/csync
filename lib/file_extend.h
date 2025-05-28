#ifndef FILE_EXTEND_H
#define FILE_EXTEND_H
#include <stdio.h>
#include "md5.h"
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <windows.h>
#elif defined(__linux__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <wchar.h>
#include <locale.h>
typedef __int64_t __int64;
#else
//others

#endif

__int64 int64_get_file_length(const char* fname);
int get_file_md5(const char* file_name, char* buf);
#endif