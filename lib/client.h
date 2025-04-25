#pragma once
#ifndef SYNC_CLIENT_H
#define SYNC_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librsync.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <json-c/json.h>
#include <tchar.h>


#include "diff.h"
#include "log.h"
#include "md5.h"
#include "protocol_sm.h"

#include <librsync.h>
#pragma comment(lib, "ws2_32.lib") 

#define SOCKET_BUF_MAX 4096
#define CHECK_SUM_LEN 32+1
#define PATH_MAX_LEN 4096

/*erro*/
#define CLIENT_ERROR_START 5000
#define CLIENT_ERROR_INITIAL_SOCKET CLIENT_ERROR_START+1
#define CLIENT_ERROR_REQ_SIG CLIENT_ERROR_START+2
#define CLIENT_ERROR_REQ_NEW CLIENT_ERROR_START+3
#define CLIENT_ERROR_SEND_NEW CLIENT_ERROR_START+4
#define CLIENT_ERROR_RECV_SIG CLIENT_ERROR_START+5
#define CLIENT_ERROR_REQ_DELTA CLIENT_ERROR_START+6
#define CLIENT_ERROR_SEND_DELTA CLIENT_ERROR_START+7
/*
enum sync_status
{
    READY_SYNC = 0,

    CLIENT_REQ_SIG = 1,
    SERVER_ACK_SIG = 2,

    CLIENT_RECV_SIG = 3,
    SERVER_SEND_SIG = 4,

    CLIENT_SEND_DEL = 5,
    SERVER_RECV_DEL = 6,

    CLIENT_SENDING_DEL = 7,
    SERVER_RECV_END = 8,
    SERVER_PATCHED = 9,

    CLIENT_SEND_NEW = 10,
    SERVER_RECVING_NEW = 11,
    SERVER_RECV_NEW_END = 12,
    ERROR_STS = 13
};

enum sync_status
{
    READY_BASE,
    PATH_SYNC,
    READY_SYNC,
    CLIENT_REQ_DIR,
    SERVER_ACK_DIR,
    CLIENT_REQ_FILE,
    SERVER_ACK_FILE,
    CLIENT_REQ_SIG,
    SERVER_ACK_SIG,
    CLIENT_RECV_SIG,
    SERVER_SEND_SIG,
    CLIENT_SEND_DEL,
    SERVER_RECV_DEL,
    CLIENT_SENDING_DEL,
    SERVER_RECV_END,
    SERVER_PATCHED,
    CLIENT_SEND_NEW,
    SERVER_RECVING_NEW,
    SERVER_RECV_NEW_END,
    ERROR_STS
};*/
int client_sync_connect(const char* server_address, int port, SOCKET* client_socket);
int client_sync_path(SOCKET client_socket, const char* instance_id);
int client_sync_file(SOCKET client_socket, const char* file_name,const char* short_name);
int client_create_dir(SOCKET client_socket, const char* dir_name);
int client_create_file(SOCKET client_socket, const char* file_name);
int client_delete_file(SOCKET client_socket, const char* file_name);
int client_rename_file(SOCKET client_socket, const char* file_name1, const char* file_name2);
int client_sync_dir(SOCKET client_socket, LPCTSTR full_dir_path, LPCTSTR dir_path);
void client_sync_close(SOCKET client_socket);

int client_req_sig(SOCKET client_socket, const char* file_name, const char* short_name, long* ack_sig_len, char* check_sum);
int client_req_dir(SOCKET client_socket, const char* dir_name);
int client_req_file(SOCKET client_socket, const char* dir_name);
int client_del_file(SOCKET client_socket, const char* dir_name);
int client_rename_file_s(SOCKET client_socket, const char* filename1, const char* filename2);
int client_recv_sig(SOCKET client_socket, const char* file_name, const char* short_name, const char* sig_name, long* ack_sig_len, char* check_sum);
int client_req_delta(SOCKET client_socket, const char* file_name, const char* short_name, const char* sig_name, const char* delta_name, long* delta_len, char* check_sum);
int client_send_delta(SOCKET client_socket,  const char* delta_name, long* delta_len);
int client_req_new(SOCKET client_socket, const char* file_name, const char* short_name, long* file_len, char* check_sum);
int client_send_new(SOCKET client_socket, const char* file_name, const char* short_name, long* file_len, char* check_sum);


int check_remove_file(const char* file_name);
int char_to_wchar(char* char_str,wchar_t* wchar_str);
int wchar_to_char(wchar_t* wchar_str, char* char_str);
wchar_t* CharToWchar(const char* charStr);
char* WideCharToMultiByteStr(const wchar_t* wideStr);

#endif