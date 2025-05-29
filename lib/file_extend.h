#ifndef FILE_EXTEND_H
#define FILE_EXTEND_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "md5.h"
#include "code.h"

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


#if defined(_WIN32) || defined(_WIN64)
int file_type_check(wchar_t* path);
int directory_clean_file(LPCTSTR dir_path);
#elif defined(__linux__)
int file_type_check(const char* path);
#else
//others
#endif
long os_update_time(long timel);
int get_time_adj_status();
/*
file
*/
__int64 file_length_int64(const char* fname);
int file_md5_hex(const char* file_name, char* buf);
int file_check_exist(const char* fname);
int file_create(const char* fname);
int file_delete(const char* fname);
long file_get_modify_time(const char* fname);
int file_update_modify_time(const char* fname, time_t f_time);
int read_file_to_buff(char* fname, char* data, long len);
void modify_os_file_name(int os_type, char* fname);
void format_file_name(char* fname);

int directory_create(const char* dname);
int directory_delete(const char* dname);
#endif