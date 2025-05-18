#include "net.h"

#if defined(_WIN32) || defined(_WIN64) // fow windwos

int start_server_iocp(const char* listen_address, int port)
{
    // initial
    initial_connection_status();
    DWORD ret;
    WSADATA wsa_data;
    SYSTEM_INFO system_info;
    DWORD thread_id;
    if ((ret = WSAStartup(0x0202, &wsa_data)) != 0)
    {
        s_log(LOG_ERROR, "csync data server WSAStartup error: %d.", ret);
        return WSAStartup_ERROR;
    }
    if ((server_info.completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
    {
        s_log(LOG_ERROR, "CreateIoCompletionPort INVALID_HANDLE_VALUE: %d.", GetLastError());
        return CREATE_PORT_INVALID_HANDLE;
    }
    GetSystemInfo(&system_info);
    s_log(LOG_DEBUG, "iocp server create %d work thread.", system_info.dwNumberOfProcessors * 2);
    for (int i = 0; i < system_info.dwNumberOfProcessors * 2; i++)
    {
        HANDLE thread_handle;
        if ((thread_handle = CreateThread(NULL, 0, iocp_server_work_thread, server_info.completion_port, 0, &thread_id)) == NULL)
        {
            s_log(LOG_ERROR, "CreateThread iocp work thread error: %d.", GetLastError());
            return CREATE_WORK_THREAD_ERROR;
        }
        CloseHandle(thread_handle);
    }

    if ((server_info.server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        s_log(LOG_ERROR, "WSASocket() error: %d.", WSAGetLastError());
        return CREATE_SERVER_SOCKET_ERROR;
    }
    else
    {
        s_log(LOG_DEBUG, "iocp server create socket..");
    }

    server_info.server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, listen_address, &server_info.server_addr.sin_addr) <= 0)
    {
        s_log(LOG_ERROR, "inet_pton failed");
        return 1;
    }

    server_info.server_addr.sin_port = htons(port);
    if (bind(server_info.server_socket, (PSOCKADDR)&server_info.server_addr, sizeof(server_info.server_addr)) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "bind() error: %d.", WSAGetLastError());
        return BIND_ERROR;
    }
    else
    {
        s_log(LOG_DEBUG, "iocp server bind socket: %d ..", server_info.server_socket);
    }

    if (listen(server_info.server_socket, 5) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "listen() error %d.", WSAGetLastError());
        return LISTEN_ERROR;
    }
    else
    {
        s_log(LOG_DEBUG, "listening on  %d .", port);
    }
    s_log(LOG_INFO, "iocp server is listening on  %s:%d .", listen_address, port);
    SOCKET accept_client;
    LPPER_HANDLE_DATA PerHandleData;
    LPPER_IO_OPERATION_DATA PerIoData;
    while (TRUE)
    {
        if ((accept_client = WSAAccept(server_info.server_socket, NULL, NULL, NULL, 0)) == SOCKET_ERROR)
        {
            s_log(LOG_ERROR, "WSAAccept()   error:   %d.", WSAGetLastError());
            return ACCEPT_ERROR;
        }

        if ((PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL)
        {
            s_log(LOG_ERROR, "GlobalAlloc()   error:   %d.", GetLastError());
            return MALLOC_ERROR;
        }
        malloc_connection_status(accept_client);
        PerHandleData->Socket = accept_client;

        if (CreateIoCompletionPort((HANDLE)accept_client, server_info.completion_port, (LPDWORD)PerHandleData, 0) == NULL)
        {
            s_log(LOG_ERROR, "CreateIoCompletionPort   error:   %d.", GetLastError());
            return CREATE_PORT_HANDLE;
        }

        if ((PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATION_DATA))) == NULL)
        {
            s_log(LOG_ERROR, "GlobalAlloc() error: %d.", GetLastError());
            return MALLOC_ERROR;
        }

        ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
        PerIoData->BytesSEND = 0;
        PerIoData->BytesRECV = 0;
        PerIoData->DataBuf.len = SOCKET_BUFFER_LEN;
        PerIoData->DataBuf.buf = PerIoData->Buffer;

        DWORD Flags = 0;
        DWORD RecvBytes;
        if (WSARecv(accept_client, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags, &(PerIoData->Overlapped), NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                s_log(LOG_ERROR, "WSARecv() error: %d.", WSAGetLastError());
                return WSARecv_ERROR;
            }
        }
    }
    return 0;
}

DWORD WINAPI iocp_server_work_thread(LPVOID iocp_id)
{
    HANDLE completion_port = (HANDLE)iocp_id;
    DWORD BytesTransferred;
    LPOVERLAPPED Overlapped;
    LPPER_HANDLE_DATA PerHandleData;
    LPPER_IO_OPERATION_DATA PerIoData;
    DWORD SendBytes, RecvBytes;
    DWORD Flags;

    while (TRUE)
    {
        if (GetQueuedCompletionStatus(completion_port, &BytesTransferred, (LPDWORD)&PerHandleData, (LPOVERLAPPED*)&PerIoData, INFINITE) == 0)
        {
            s_log(LOG_INFO, "GetQueuedCompletionStatus   error: %d.", GetLastError());
            return 0;
        }

        if (BytesTransferred == 0)
        {
            s_log(LOG_DEBUG, "closing socket   %d.", PerHandleData->Socket);
            reset_connection_status(PerHandleData->Socket);
            if (closesocket(PerHandleData->Socket) == SOCKET_ERROR)
            {
                s_log(LOG_ERROR, "closesocket()   error: %d.", WSAGetLastError());
                return 0;
            }

            GlobalFree(PerHandleData);
            GlobalFree(PerIoData);
            continue;
        }

        if (PerIoData->BytesRECV == 0)
        {
            PerIoData->BytesRECV = BytesTransferred;
            PerIoData->BytesSEND = 0;
        }
        else
        {
            PerIoData->BytesSEND += BytesTransferred;
            PerIoData->BytesRECV = 0;
        }

        if (PerIoData->BytesRECV > PerIoData->BytesSEND)
        {
            ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
            PerIoData->DataBuf.buf = PerIoData->Buffer + PerIoData->BytesSEND;
            PerIoData->DataBuf.len = PerIoData->BytesRECV - PerIoData->BytesSEND;
            PerIoData->DataBuf.buf[PerIoData->DataBuf.len] = 0;
            long con_index = get_socket_connection_index(PerHandleData->Socket);
            while (1)
            {
                if (1 == push_stream_to_data(PerIoData->DataBuf.buf, PerIoData->DataBuf.len, &conn_status[con_index]))
                {
                    if (conn_status[con_index].data_len > 0)
                    {
                        PerIoData->DataBuf.buf = conn_status[con_index].data;
                        PerIoData->DataBuf.len = conn_status[con_index].data_len;
                        if (WSASend(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &SendBytes, 0,
                            &(PerIoData->Overlapped), NULL) == SOCKET_ERROR)
                        {
                            if (WSAGetLastError() != ERROR_IO_PENDING)
                            {
                                s_log(LOG_ERROR, "WSASend() error:   %d.", WSAGetLastError());
                                return 0;
                            }
                        }
                    }
                    else
                    {
                        PerIoData->BytesRECV = 0;
                        Flags = 0;
                        ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
                        PerIoData->DataBuf.len = SOCKET_BUFFER_LEN;
                        PerIoData->DataBuf.buf = PerIoData->Buffer;
                        if (WSARecv(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags,
                            &(PerIoData->Overlapped), NULL) == SOCKET_ERROR)
                        {
                            if (WSAGetLastError() != ERROR_IO_PENDING)
                            {
                                s_log(LOG_ERROR, "WSARecv() error:   %d.", WSAGetLastError());
                                return 0;
                            }
                        }
                    }
                    post_update_status(&conn_status[con_index]);
                    break;
                }
            }
        }
        else
        {
            PerIoData->BytesRECV = 0;
            Flags = 0;
            ZeroMemory(&(PerIoData->Overlapped), sizeof(OVERLAPPED));
            PerIoData->DataBuf.len = SOCKET_BUFFER_LEN;
            PerIoData->DataBuf.buf = PerIoData->Buffer;
            if (WSARecv(PerHandleData->Socket, &(PerIoData->DataBuf), 1, &RecvBytes, &Flags, &(PerIoData->Overlapped), NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    s_log(LOG_ERROR, "WSARecv() error:   %d.", WSAGetLastError());
                    return 0;
                }
            }
        }
    }
}

#elif defined(__linux__)
// linux
int start_server_linux(const char* listen_address, int port)
{
    initial_connection_status();
    int server_fd, new_socket;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        s_log(LOG_ERROR, "create socket error.");
        return CREATE_SERVER_SOCKET_ERROR;
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(listen_address);
    address.sin_port = htons(port);
    int addrlen = sizeof(address);
    if (inet_pton(AF_INET, listen_address, &address.sin_addr) <= 0)
    {
        s_log(LOG_ERROR, "inet_pton socket error.");
        return CREATE_SERVER_SOCKET_ERROR;
    }

    if (bind(server_fd, (struct sockaddr*)&(address), sizeof(address)) < 0)
    {
        s_log(LOG_ERROR, "bind socket error.");
        return BIND_ERROR;
    }

    if (listen(server_fd, MAX_CONNECTION_NUM) < 0)
    {
        s_log(LOG_ERROR, "listen socket error.");
        return LISTEN_ERROR;
    }
    while (1)
    {
        new_socket = accept(server_fd, (struct sockaddr*)&(address), (socklen_t*)&(addrlen));
        if (new_socket < 0)
        {
            s_log(LOG_ERROR, "accept error.");
            return ACCEPT_ERROR;
        }
        else
        {
            malloc_connection_status(new_socket);
            pthread_t accept_thread;
            if (pthread_create(&accept_thread, NULL, linux_server_work_thread, (void*)&new_socket) < 0)
            {
                s_log(LOG_ERROR, "accept thread error.");
                return ACCEPT_ERROR;
            }
        }

    }
    return 0;
}

void* linux_server_work_thread(void* socket_desc)
{
    int* socket = (int*)socket_desc;
    int sock = *socket;
    char buffer[4096] = { 0 };
    int i = 0;
    struct timespec ts;
    long con_index = get_socket_connection_index(sock);
    while (1)
    {
        int valread = read(sock, buffer, 4096);
        if (valread <= 0)
        {
            s_log(LOG_ERROR, "client closed for instance: %s.", conn_status[con_index].instance_id);
            break;
        }
        while (1)
        {
            if (1 == push_stream_to_data(buffer, valread, &conn_status[con_index]))
            {
                if (conn_status[con_index].data_len > 0)
                {
                    send(sock, conn_status[con_index].data, conn_status[con_index].data_len, 0);
                }
                post_update_status(&conn_status[con_index]);
                break;
            }
        }
    }
    close(sock);
}
#else
//others
#endif

int get_peer_address(SOCKET client_socket, char* client_address, int* port)
{
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    if (getpeername(client_socket, (struct sockaddr*)&client_addr, &client_addr_len) == 0)
    {
        inet_ntop(AF_INET, &client_addr.sin_addr, client_address, IPV4_ADDRESS_LEN);
        *port = ntohs(client_addr.sin_port);
        return 0;
    }
    else
    {
        printf("Failed to get client address: %d\n", WSAGetLastError());
        return 1;
    }
}

void initial_connection_status()
{
    for (long i = 0;i < MAX_CONNECTION_NUM;i++)
    {
        conn_status[i].index = i;
        conn_status[i].socket = 0;
        conn_status[i].status = READY_BASE;
        conn_status[i].using_status = 0;
        memset(conn_status[i].cache_path, 0, FILE_NAME_MAX_LENGTH);
        sprintf_s(conn_status[i].cache_path, FILE_NAME_MAX_LENGTH, "cache\\");
    }
}

long get_idle_connection_index()
{
    for (long i = 0;i < MAX_CONNECTION_NUM;i++)
    {
        if (conn_status[i].using_status == 0)
        {
            return i;
        }

    }
    return -1;
}

long get_socket_connection_index(int socket)
{
    for (long i = 0;i < MAX_CONNECTION_NUM;i++)
    {
        if (conn_status[i].socket == socket)
        {
            return i;
        }
    }
    return -1;
}

long malloc_connection_status(int socket)
{
    long index = get_idle_connection_index();
    conn_status[index].socket = socket;
    conn_status[index].status = 0;
    conn_status[index].using_status = 1;
    conn_status[index].big_cache_counts = 0;
    conn_status[index].big_cache_malloc = 0;
    get_peer_address(socket, conn_status[index].client_address, &(conn_status[index].client_port));
    s_log(LOG_INFO, "client %s:%d connected.", conn_status[index].client_address, conn_status[index].client_port);
    memset(conn_status[index].sig_name, 0, FILE_NAME_MAX_LENGTH);
    sprintf_s(conn_status[index].sig_name, FILE_NAME_MAX_LENGTH, "%s%s%d", conn_status[index].cache_path, "sig", socket);
    memset(conn_status[index].delta_name, 0, FILE_NAME_MAX_LENGTH);
    sprintf_s(conn_status[index].delta_name, FILE_NAME_MAX_LENGTH, "%s%s%d", conn_status[index].cache_path, "del", socket);

    return index;
}

void reset_connection_status(int socket)
{
    for (long i = 0;i < MAX_CONNECTION_NUM;i++)
    {
        if (conn_status[i].socket == socket)
        {
            conn_status[i].socket = 0;
            conn_status[i].status = READY_SYNC;
            conn_status[i].using_status = 0;
            if (conn_status[i].data_len > 0)
            {
                conn_status[i].data_len = 0;
                free(conn_status[i].data);
            }
            if (conn_status[i].next_data_len > 0)
            {
                conn_status[i].next_data_len = 0;
                free(conn_status[i].next_data);
            }
        }
    }
}
