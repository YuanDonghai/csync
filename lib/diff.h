#ifndef SYNC_DIFF_H
#define SYNC_DIFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librsync.h>
#include <stdarg.h>
#include <stdint.h>

#include "log.h"
#include "file_extend.h"

#if defined(_WIN32) || defined(_WIN64)

#elif defined(__linux__)
// linux
typedef __int64_t __int64;
#else
//others
#endif

#define LEVEL_FILE_SIZE_4K 4*1024
#define LEVEL_FILE_SIZE_4M LEVEL_FILE_SIZE_4K*1024
#define LEVEL_FILE_SIZE_64M LEVEL_FILE_SIZE_4M*16
#define LEVEL_FILE_SIZE_512M LEVEL_FILE_SIZE_4M*128
#define LEVEL_FILE_SIZE_1G LEVEL_FILE_SIZE_4M*256

rs_result rdiff_sig(const char* basis_name, const char* sig_name);
rs_result rdiff_delta(const char* newfile_name, const char* sig_name, const char* delta_name);
rs_result rdiff_patch(const char* basis_name, const char* delta_name, const char* new_filename);

int64_t diff_get_file_length(char* fname);
#endif