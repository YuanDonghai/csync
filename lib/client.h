#ifndef SYNC_CLIENT_H
#define SYNC_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <librsync.h>
#include <json-c/json.h>

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#elif defined(__linux__)
// linux
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET  (int)(~0)
#define SOCKET_ERROR            (-1)

#else
//others
#endif


#include "diff.h"
#include "log.h"
#include "md5.h"
#include "protocol_sm.h"
#include "code.h"
#include "csync_error.h"


#define SOCKET_BUF_MAX 4096
#define CHECK_SUM_LEN 32+1
#define PATH_MAX_LEN 4096


#if defined(_WIN32) || defined(_WIN64)
int client_sync_dir(SOCKET client_socket, LPCTSTR full_dir_path, LPCTSTR dir_path);
#elif defined(__linux__)
// linux
int WSAGetLastError();
int client_sync_dir(SOCKET client_socket, const char* full_dir_path, const char* dir_path);
#else
//others
#endif



int client_sync_connect(const char* server_address, int port, SOCKET* client_socket);
int client_sync_path(SOCKET client_socket, const char* instance_id);
int client_sync_file(SOCKET client_socket, const char* file_name, const char* short_name);
int client_create_dir(SOCKET client_socket, const char* dir_name);
int client_create_file(SOCKET client_socket, const char* file_name);
int client_delete_file(SOCKET client_socket, const char* file_name);
int client_rename_file(SOCKET client_socket, const char* file_name1, const char* file_name2);
void client_sync_close(SOCKET client_socket);

int client_req_sig(SOCKET client_socket, const char* file_name, const char* short_name, long* ack_sig_len, char* check_sum);
int client_req_dir(SOCKET client_socket, const char* dir_name);
int client_req_file(SOCKET client_socket, const char* dir_name);
int client_del_file(SOCKET client_socket, const char* dir_name);
int client_rename_file_s(SOCKET client_socket, const char* filename1, const char* filename2);
int client_recv_sig(SOCKET client_socket, const char* file_name, const char* short_name, const char* sig_name, long* ack_sig_len, char* check_sum);
int client_req_delta(SOCKET client_socket, const char* file_name, const char* short_name, const char* sig_name, const char* delta_name, long* delta_len, char* check_sum);
int client_send_delta(SOCKET client_socket, const char* delta_name, long* delta_len);
int client_req_new(SOCKET client_socket, const char* file_name, const char* short_name, long* file_len, char* check_sum);
int client_send_new(SOCKET client_socket, const char* file_name, const char* short_name, long* file_len, char* check_sum);


int check_remove_file(const char* file_name);

#endif