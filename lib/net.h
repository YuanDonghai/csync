#ifndef SYNC_SERVER_H
#define SYNC_SERVER_H
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <windows.h>
#elif defined(__linux__)
// linux
#else
//others
#endif

#include "csync_error.h"
#include "protocol.h"
#include "log.h"
#define MAX_CONNECTION_NUM 65535
#define SOCKET_BUFFER_LEN 4096
#pragma comment(lib, "ws2_32.lib")

typedef struct
{
    OVERLAPPED Overlapped;
    WSABUF DataBuf;
    CHAR Buffer[SOCKET_BUFFER_LEN];
    DWORD BytesSEND;
    DWORD BytesRECV;
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

typedef struct
{
    SOCKET Socket;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

enum server_io_type
{
    SELECT = 0,
    IOCP = 1
};

typedef struct
{
    SOCKET server_socket;
    SOCKADDR_IN server_addr;
    HANDLE completion_port;
} WIN_SERVER_GLOBAL, *LPWIN_SERVER_GLOBAL;



static WIN_SERVER_GLOBAL server_info;
static sync_protocol conn_status[MAX_CONNECTION_NUM];

int start_server(const char *listen_address, int port, enum server_io_type type);
int start_server_iocp(const char *listen_address, int port);

/* iocp functions*/
DWORD WINAPI iocp_server_work_thread(LPVOID iocp_id);

/* socket extend*/
int get_peer_address(SOCKET client_socket, char* client_address, int* port);
/* protocol */
void initial_connection_status();
long malloc_connection_status(int socket);
void reset_connection_status(int socket);
long get_idle_connection_index();
long get_socket_connection_index(int socket);

#endif