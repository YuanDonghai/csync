#ifndef CODE_H
#define CODE_H
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

void TCHARToChar(const TCHAR *tcharStr, char *charStr, size_t charStrSize);
void trans_hex_to_ascii(char* ch_in, int len, char* ch_out);
void trans_ascii_to_hex(char* ch_in, int len, char* ch_out);

char* gen_node_id();
void format_path(char* path);
#endif