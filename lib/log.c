#include "log.h"

int set_log_params(char* path, int format, enum log_level_def level)
{
    memset(log_path, 0, LOG_FILE_NAME_LENGTH);
    sprintf_s(log_path, LOG_FILE_NAME_LENGTH,"%s", path);
    log_level = level;
}

void s_log(enum log_level_def level, const char* data, ...)
{
    if ((level > log_level) || (level < 0))
    {
        return;
    }
    time_t now_t;
    time(&now_t);

    memset(log_data, 0, SINGLE_LOG_MAX_LENGTH);

    char cur_time[256];
    memset(cur_time, 0, 256);
    struct tm* t = localtime(&now_t);
    strftime(cur_time, 256, "%Y-%m-%d %H:%M:%S", t);
    
    sprintf_s(log_data, SINGLE_LOG_MAX_LENGTH, "[%s][%s] ", cur_time, log_level_string[level]);
    va_list args;
    va_start(args,data);
    vsnprintf(log_data+strlen(log_data),sizeof(log_data)-strlen(log_data),data,args);
    va_end(args);
    strcat_s(log_data, SINGLE_LOG_MAX_LENGTH,"\n");

    printf("%s",log_data);

/*
    if (f_log)
    {
        fwrite(log_data,strlen(log_data),1,f_log);
    }
    else
    {
        f_log=fopen(log_path, "a");
        fwrite(log_data, strlen(log_data), 1, f_log);
    }
*/
}