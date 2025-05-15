#ifndef CODE_H
#define CODE_H

#include <stdio.h>
#include <json-c/json.h>

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
#else
//others
#endif

#define FILE_PATH_MAX_LEN 4096
#define IPV4_ADDRESS_LEN 16
#define INSTANCE_ID_LEN 64


#if defined(_WIN32) || defined(_WIN64)
int char_to_wchar(const char* char_str, wchar_t* wchar_str, int w_len, int code);
int wchar_to_char(wchar_t* wchar_str, const char* char_str, int w_len, int code);
int char_to_char(char* char_str1, char* char_str2, int c_len, int code1, int code2);

#elif defined(__linux__)
// linux
void sprintf_s(char * buffer,size_t size_of_buffer,const char * format,...);
void strcat_s(char* dst_buf, size_t size_of_buffer, char* src_buf);
#else
//others
#endif

const char* trans_char_to_local(char* char_str);
const char* trans_char_to_utf8(char* char_str);
void os_char_to_utf8(char* char_str, char* ch_out);

void trans_hex_to_ascii(char* ch_in, int len, char* ch_out);
void trans_ascii_to_hex(char* ch_in, int len, char* ch_out);

const char* gen_uuid_str();
const char* get_hostname();
void format_path(char* path);
int load_file_to_json(struct json_object** json_data, const char* file_path);
int dump_json_to_file(struct json_object* json_data, const char* file_path);

void _sleep_or_Sleep(int ms);
static char uuid_ch[39];
#endif