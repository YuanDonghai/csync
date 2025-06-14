#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "code.h"

#define LOG_FILE_NAME_LENGTH 512
#define SINGLE_LOG_MAX_LENGTH 4096*4

enum log_level_def
{
    LOG_ERROR = 0,
    LOG_INFO = 1,
    LOG_DEBUG = 2
};

int set_log_params(char* path, int format, enum log_level_def level);
void s_log(enum log_level_def level, const char* data, ...);
#endif