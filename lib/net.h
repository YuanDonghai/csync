#ifndef SYNC_SERVER_H
#define SYNC_SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#else
//others
#endif

#include "csync_error.h"
#include "protocol.h"
#include "log.h"
#include "code.h"
#include "config.h"
#include "instance.h"

#define MAX_CONNECTION_NUM 65535
#define SOCKET_BUFFER_LEN 4096

#if defined(_WIN32) || defined(_WIN64)
// windows iocp
typedef struct
{
    OVERLAPPED Overlapped;
    WSABUF DataBuf;
    CHAR Buffer[SOCKET_BUFFER_LEN];
    DWORD BytesSEND;
    DWORD BytesRECV;
} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

typedef struct
{
    SOCKET Socket;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct
{
    SOCKET server_socket;
    SOCKADDR_IN server_addr;
    HANDLE completion_port;
} WIN_SERVER_GLOBAL, * LPWIN_SERVER_GLOBAL;

static WIN_SERVER_GLOBAL server_info;
// iocp server
int start_server_iocp(const char* listen_address, int port);
DWORD WINAPI iocp_server_work_thread(LPVOID iocp_id);

#elif defined(__linux__)
// linux
typedef int SOCKET;
int start_server_linux(const char* listen_address, int port);
void* linux_server_work_thread(void* socket_desc);
#else
//others
#endif

/*static sync flags*/
static sync_protocol conn_status[MAX_CONNECTION_NUM];
/* socket extend*/
int get_peer_address(SOCKET client_socket, char* client_address, int* port);
/* protocol */
void initial_connection_status();
long malloc_connection_status(int socket);
void reset_connection_status(int socket);
long get_idle_connection_index();
long get_socket_connection_index(int socket);

#endif