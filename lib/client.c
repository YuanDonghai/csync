#include "client.h"


#if defined(_WIN32) || defined(_WIN64)
int client_sync_dir(SOCKET client_socket, LPCTSTR full_dir_path, LPCTSTR dir_path)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR searchPath[FILE_PATH_MAX_LEN];
    TCHAR fullPath[FILE_PATH_MAX_LEN];
    TCHAR shortPath[FILE_PATH_MAX_LEN];

    char ch_path[FILE_PATH_MAX_LEN];
    char short_name[FILE_PATH_MAX_LEN];
    _stprintf_s(searchPath, FILE_PATH_MAX_LEN, _T("%s\\*"), full_dir_path);
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    while (FindNextFile(hFind, &findFileData) != 0)
    {
        const TCHAR* name = findFileData.cFileName;
        if (_tcscmp(name, _T(".")) == 0 || _tcscmp(name, _T("..")) == 0)
        {
            continue;
        }
        _stprintf_s(fullPath, FILE_PATH_MAX_LEN, _T("%s\\%s"), full_dir_path, name);
        _stprintf_s(shortPath, FILE_PATH_MAX_LEN, _T("%s\\%s"), dir_path, name);
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            memset(ch_path, 0, FILE_PATH_MAX_LEN);
            wchar_to_char(shortPath, ch_path);
            client_create_dir(client_socket, ch_path);
            client_sync_dir(client_socket, fullPath, shortPath);
        }
        else
        {
            memset(ch_path, 0, FILE_PATH_MAX_LEN);
            wchar_to_char(fullPath, ch_path);
            memset(short_name, 0, FILE_PATH_MAX_LEN);
            wchar_to_char(shortPath, short_name);
            if (findFileData.nFileSizeHigh == 0 && findFileData.nFileSizeLow == 0)
            {
                client_create_file(client_socket, short_name);
            }
            else
            {
                client_sync_file(client_socket, ch_path, short_name);
            }
        }
    }

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        _tprintf(_T("Error: FindNextFile failed with error %lu\n"), dwError);
    }
    FindClose(hFind);
    return 0;
}

int client_sync_connect(const char* server_address, int port, SOCKET* client_socket)
{
    /*
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        s_log(LOG_ERROR, "[client] WSAStartup failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_INITIAL_SOCKET;
    }
    */
   s_log(LOG_INFO,"connect %s %d",server_address,client_socket);
    struct sockaddr_in serverAddr;
    *client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*client_socket == INVALID_SOCKET)
    {
        s_log(LOG_ERROR, "[client] Socket creation failed: %d.", WSAGetLastError());
        WSACleanup();
        return CLIENT_ERROR_INITIAL_SOCKET;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (InetPtonW(AF_INET, CharToWchar(server_address), &serverAddr.sin_addr) <= 0)
    {
        s_log(LOG_ERROR, "[client] Invalid address/ Address not supported .");
        closesocket(*client_socket);
        WSACleanup();
        return CLIENT_ERROR_INITIAL_SOCKET;
    }
    if (connect(*client_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] Connection failed: %d.", WSAGetLastError());
        closesocket(*client_socket);
        WSACleanup();
        return CLIENT_ERROR_INITIAL_SOCKET;
    }
    s_log(LOG_DEBUG, "[client] connection server: %s:%d.", server_address, port);
    return 0;
}

#elif defined(__linux__)
// linux
int WSAGetLastError()
{
    return -1;
}
int client_sync_dir(SOCKET client_socket, const char* full_dir_path, const char* dir_path)
{
    return 0;
}

int client_sync_connect(const char* server_address, int port, SOCKET* client_socket)
{
    struct sockaddr_in server_addr;

    *client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*client_socket < 0)
    {
        s_log(LOG_ERROR, "create socket error .");
        return CLIENT_ERROR_INITIAL_SOCKET;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_address);

    if (connect(*client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        s_log(LOG_ERROR, "connect socket error .");
        close(*client_socket);
        return CLIENT_ERROR_INITIAL_SOCKET;
    }
    return 0;
}

#else
//others
#endif



int client_sync_path(SOCKET client_socket, const char* instance_id)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];

    memset(ch_send, 0, SOCKET_BUF_MAX);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"id\" : \"%s\"}", PATH_SYNC, instance_id);
    s_log(LOG_DEBUG, "[client] client set sync path for instance %s.", instance_id);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed.");
        return CLIENT_ERROR_REQ_NEW;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    long recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_sync_path error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_REQ_NEW;
            }
        }
        json_object_put(parsed_json);
    }
    else
    {
        s_log(LOG_ERROR, "[client] client_sync_path Recv failed: .");
        return CLIENT_ERROR_REQ_NEW;
    }
    return 0;
}

int client_sync_file(SOCKET client_socket, const char* file_name, const char* short_name)
{
    long ack_sig_len = 0;
    long delta_len = 0;
    long new_file_len = 0;
    char trans_check_sum[CHECK_SUM_LEN];
    memset(trans_check_sum, 0, CHECK_SUM_LEN);
    char sig_file_name[32];
    char delta_file_name[32];
    memset(sig_file_name, 0, 32);
    memset(delta_file_name, 0, 32);
    sprintf_s(sig_file_name, 32, "%d_sig", client_socket);
    sprintf_s(delta_file_name, 32, "%d_delta", client_socket);

    if (0 != client_req_sig(client_socket, file_name, short_name, &ack_sig_len, trans_check_sum))
    {
        s_log(LOG_ERROR, "[client] client requre signature error.");
        return CLIENT_ERROR_REQ_SIG;
    }
    if (ack_sig_len == 0)
    {
        s_log(LOG_DEBUG, "[client] client will send new file.");
        memset(trans_check_sum, 0, CHECK_SUM_LEN);
        if (0 != client_req_new(client_socket, file_name, short_name, &new_file_len, trans_check_sum))
        {
            s_log(LOG_ERROR, "[client] client requre new file error.");
            return CLIENT_ERROR_REQ_NEW;
        }
        memset(trans_check_sum, 0, CHECK_SUM_LEN);
        if (0 != client_send_new(client_socket, file_name, short_name, &new_file_len, trans_check_sum))
        {
            s_log(LOG_ERROR, "[client] client send new file error.");
            return CLIENT_ERROR_SEND_NEW;
        }
    }
    else
    {        
        if (0 != client_recv_sig(client_socket, file_name, short_name, sig_file_name, &ack_sig_len, trans_check_sum))
        {
            s_log(LOG_ERROR, "[client] client recv sig error.");
            check_remove_file(sig_file_name);
            return CLIENT_ERROR_RECV_SIG;
        }
        if (0 != client_req_delta(client_socket, file_name, short_name, sig_file_name, delta_file_name, &delta_len, trans_check_sum))
        {
            s_log(LOG_ERROR, "[client] client req delta error.");
            check_remove_file(sig_file_name);
            check_remove_file(delta_file_name);
            return CLIENT_ERROR_REQ_DELTA;
        }
        if (delta_len > 9)
        {
            if (0 != client_send_delta(client_socket, delta_file_name, &delta_len))
            {
                s_log(LOG_ERROR, "[client] client send delta error.");
                check_remove_file(sig_file_name);
                check_remove_file(delta_file_name);
                return CLIENT_ERROR_SEND_DELTA;
            }
        }
        check_remove_file(sig_file_name);
        check_remove_file(delta_file_name);
    }

    return 0;
}

int client_create_dir(SOCKET client_socket, const char* dir_name)
{
    if (0 != client_req_dir(client_socket, dir_name))
    {
        s_log(LOG_ERROR, "[client] client requre signature error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}
int client_create_file(SOCKET client_socket, const char* file_name)
{
    if (0 != client_req_file(client_socket, file_name))
    {
        s_log(LOG_ERROR, "[client] client requre signature error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}
int client_delete_file(SOCKET client_socket, const char* file_name)
{
    if (0 != client_del_file(client_socket, file_name))
    {
        s_log(LOG_ERROR, "[client] client requre signature error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}

int client_rename_file(SOCKET client_socket, const char* file_name1, const char* file_name2)
{
    if (0 != client_rename_file_s(client_socket, file_name1, file_name2))
    {
        s_log(LOG_ERROR, "[client] client requre signature error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}



void client_sync_close(SOCKET client_socket)
{
#if defined(_WIN32) || defined(_WIN64)
    closesocket(client_socket);
    WSACleanup();
#elif defined(__linux__)
    // linux
    close(client_socket);
#else
    //others
#endif
    
}

int client_req_sig(SOCKET client_socket, const char* file_name, const char* short_name, long* ack_sig_len, char* check_sum)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    struct json_object* parsed_json;

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_REQ_SIG));
    json_object_object_add(node_new, "file", json_object_new_string(short_name));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    //  sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"dir\" : \"%s\"}", CLIENT_REQ_DIR, dir_name);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);

    //memset(ch_send, 0, SOCKET_BUF_MAX);
   // sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"file\" : \"%s\"}", CLIENT_REQ_SIG, file_name);
    s_log(LOG_DEBUG, "[client] client request signature. : %s.", ch_send);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed.");
        return CLIENT_ERROR_REQ_SIG;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    int recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        s_log(LOG_DEBUG, "[client] server ack signature : %s.", ch_recv);
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jtype;
        if (json_object_object_get_ex(parsed_json, "type", &jtype))
        {
            if (SERVER_ACK_SIG == json_object_get_int(jtype))
            {
                struct json_object* sig_len;
                if (json_object_object_get_ex(parsed_json, "length", &sig_len))
                {
                    *ack_sig_len = json_object_get_int(sig_len);
                }

                struct json_object* sig_sum;
                if (json_object_object_get_ex(parsed_json, "checksum", &sig_sum))
                {
                    memcpy(check_sum, json_object_get_string(sig_sum), strlen(json_object_get_string(sig_sum)));
                }
            }
            else
            {
                s_log(LOG_ERROR, "[client] server ack is not SERVER_ACK_SIG(%d): %d.", SERVER_ACK_SIG, WSAGetLastError());
                json_object_put(parsed_json);
                return CLIENT_ERROR_REQ_SIG;
            }
        }
        else
        {
            s_log(LOG_ERROR, "[client] server ack data format error.");
            return CLIENT_ERROR_REQ_SIG;
        }
    }
    else
    {
        s_log(LOG_ERROR, "[client] client recv data error.");
        return CLIENT_ERROR_REQ_SIG;
    }
    json_object_put(parsed_json);
   
    return 0;
}

int client_req_dir(SOCKET client_socket, const char* dir_name)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    struct json_object* parsed_json;

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_REQ_DIR));
    json_object_object_add(node_new, "dir", json_object_new_string(dir_name));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    //  sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"dir\" : \"%s\"}", CLIENT_REQ_DIR, dir_name);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);

    s_log(LOG_DEBUG, "[client] client request dir. : %s.", ch_send);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_SIG;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    int recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_req_dir data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_SEND_NEW;
            }
        }
        json_object_put(parsed_json);

    }
    else
    {
        s_log(LOG_ERROR, "[client] client recv data error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}

int client_req_file(SOCKET client_socket, const char* dir_name)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    struct json_object* parsed_json;

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_REQ_FILE));
    json_object_object_add(node_new, "file", json_object_new_string(dir_name));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    //  sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"dir\" : \"%s\"}", CLIENT_REQ_DIR, dir_name);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);

    s_log(LOG_DEBUG, "[client] client request dir. : %s.", ch_send);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_SIG;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    int recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_req_file data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_SEND_NEW;
            }
        }
        json_object_put(parsed_json);

    }
    else
    {
        s_log(LOG_ERROR, "[client] client recv data error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}

int client_del_file(SOCKET client_socket, const char* file_name)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    struct json_object* parsed_json;

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_DEL_FILE));
    json_object_object_add(node_new, "file", json_object_new_string(file_name));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);

    s_log(LOG_DEBUG, "[client] client delete dir or file. %s.", ch_send);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_SIG;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    int recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client delete dir or file error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_SEND_NEW;
            }
        }
        json_object_put(parsed_json);

    }
    else
    {
        s_log(LOG_ERROR, "[client] client recv data error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}

int client_rename_file_s(SOCKET client_socket, const char* filename1, const char* filename2)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    struct json_object* parsed_json;

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_RENAME_FILE));
    json_object_object_add(node_new, "file1", json_object_new_string(filename1));
    json_object_object_add(node_new, "file2", json_object_new_string(filename2));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    //  sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"dir\" : \"%s\"}", CLIENT_REQ_DIR, dir_name);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);

    s_log(LOG_DEBUG, "[client] client request dir. : %s.", ch_send);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_SIG;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    int recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_rename_file_s data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_SEND_NEW;
            }
        }
        json_object_put(parsed_json);

    }
    else
    {
        s_log(LOG_ERROR, "[client] client recv data error.");
        return CLIENT_ERROR_REQ_SIG;
    }

    return 0;
}

int client_recv_sig(SOCKET client_socket, const char* file_name, const char* short_name, const char* sig_name, long* ack_sig_len, char* check_sum)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_RECV_SIG));
    json_object_object_add(node_new, "file", json_object_new_string(short_name));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    //  sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"dir\" : \"%s\"}", CLIENT_REQ_DIR, dir_name);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);

    //   memset(ch_send, 0, SOCKET_BUF_MAX);
     //  sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"file\" : \"%s\"}", CLIENT_RECV_SIG, file_name);
    s_log(LOG_DEBUG, "[client] client recv signature: data=%s.", ch_send);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_RECV_SIG;
    }
    long recv_sum = 0;
    FILE* file = fopen(sig_name, "wb");
    if (!file)
    {
        s_log(LOG_ERROR, "[client] config open file error. %s.", sig_name);
        return CLIENT_ERROR_RECV_SIG;
    }
    long  recv_len = 0;
    while (recv_sum < *ack_sig_len)
    {
        memset(ch_recv, 0, SOCKET_BUF_MAX);
        recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
        if (recv_len > 0)
        {
            fwrite(ch_recv, recv_len, 1, file);
            recv_sum += recv_len;
        }
        else
        {
            break;
        }
    }
    fclose(file);
    int res = check_file_with_md5(sig_name, check_sum);
    if (0 != res)
    {
        s_log(LOG_ERROR, "[client] client_recv_sig signature file checksum error.");
        return CLIENT_ERROR_RECV_SIG;
    }
    s_log(LOG_DEBUG, "[client] client recv signature: length=%d.", *ack_sig_len);
    return 0;
}

int client_req_delta(SOCKET client_socket, const char* file_name, const char* short_name, const char* sig_name, const char* delta_name, long* delta_len, char* check_sum)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    
    rs_result res = rdiff_delta(file_name, sig_name, delta_name);
    if (res != 0)
    {
        s_log(LOG_ERROR, "[client] gen delta file error");
        return CLIENT_ERROR_REQ_DELTA;
    }
    FILE* file = fopen(delta_name, "rb");
    if (!file)
    {
        s_log(LOG_ERROR, "[client] config open file error. %s.", delta_name);
        return CLIENT_ERROR_REQ_DELTA;
    }
    fseek(file, 0, SEEK_END);
    long lend = ftell(file);
    *delta_len = lend;
    fseek(file, 0, SEEK_SET);
    char ch_out[32];
    char ch_out_hex[64];
    memset(ch_out, 0, 32);
    md5_stream(file, ch_out);
    fclose(file);
    trans_ascii_to_hex(ch_out, 32, ch_out_hex);
    ch_out_hex[32] = 0;

    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_SEND_DEL));
    json_object_object_add(node_new, "length", json_object_new_int(lend));
    json_object_object_add(node_new, "checksum", json_object_new_string(ch_out_hex));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);
    s_log(LOG_DEBUG, "[client] client request send delta. data: %s", ch_send);
    // memset(ch_send, 0, SOCKET_BUF_MAX);
    // sprintf_s(ch_send, SOCKET_BUF_MAX, "{\"type\": %d,\"length\" : %d, \"checksum\" : \"%s\"}", CLIENT_SEND_DEL, lend, ch_out_hex);
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_DELTA;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    long recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_req_delta  data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_REQ_DELTA;
            }
        }
        json_object_put(parsed_json);
    }
    else
    {
        s_log(LOG_ERROR, "[client]client_req_delta Recv failed : % d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_DELTA;
    }
    s_log(LOG_DEBUG, "[client] server is ready to recv delta.");
    return 0;
}

int client_send_delta(SOCKET client_socket, const char* delta_name, long* delta_len)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    FILE* filers = fopen(delta_name, "rb");
    if (!filers)
    {
        s_log(LOG_ERROR, "[client] config open file error. %s.", delta_name);
        return CLIENT_ERROR_SEND_DELTA;
    }
    fseek(filers, 0, SEEK_SET);
    long read_len = 0;
    for (long i = 0;i < *delta_len;i = i + SOCKET_BUF_MAX)
    {
        memset(ch_send, 0, SOCKET_BUF_MAX);
        read_len = fread(ch_send, 1, SOCKET_BUF_MAX, filers);
        if (send(client_socket, ch_send, read_len, 0) == SOCKET_ERROR)
        {
            s_log(LOG_ERROR, "[client] Send failed: %d.", WSAGetLastError());
            return CLIENT_ERROR_SEND_DELTA;
        }
    }
    fclose(filers);
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    long recvLen = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recvLen > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_req_delta data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_SEND_DELTA;
            }
        }
        json_object_put(parsed_json);
    }
    else
    {
        s_log(LOG_ERROR, "[client] client_send_delta Recv failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_SEND_DELTA;
    }
    return 0;
}

int client_req_new(SOCKET client_socket, const char* file_name, const char* short_name, long* file_len, char* check_sum)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        s_log(LOG_ERROR, "[client] config open file error. %s.", file_name);
        return CLIENT_ERROR_REQ_NEW;
    }
    fseek(file, 0, SEEK_END);
    long lend = ftell(file);
    *file_len = lend;

    fseek(file, 0, SEEK_SET);
    char ch_out[32];
    char ch_out_hex[64];
    memset(ch_out, 0, 32);
    md5_stream(file, ch_out);   
    fclose(file);
    trans_ascii_to_hex(ch_out, 32, ch_out_hex);
    ch_out_hex[32] = 0;
    struct json_object* node_new = json_object_new_object();
    json_object_object_add(node_new, "type", json_object_new_int(CLIENT_SEND_NEW));
    json_object_object_add(node_new, "length", json_object_new_int(lend));
    json_object_object_add(node_new, "checksum", json_object_new_string(ch_out_hex));
    memset(ch_send, 0, SOCKET_BUF_MAX);
    sprintf_s(ch_send, SOCKET_BUF_MAX, "%s", json_object_get_string(node_new));
    json_object_put(node_new);
    // memset(ch_send, 0, SOCKET_BUF_MAX);  
    if (send(client_socket, ch_send, strlen(ch_send), 0) == SOCKET_ERROR)
    {
        s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_NEW;
    }
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    long recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_req_new data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_REQ_NEW;
            }
        }
        json_object_put(parsed_json);
    }
    else
    {
        s_log(LOG_ERROR, "[client] client_req_new Recv failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_REQ_NEW;
    }
    return 0;
}

int client_send_new(SOCKET client_socket, const char* file_name, const char* short_name, long* file_len, char* check_sum)
{
    char ch_send[SOCKET_BUF_MAX];
    char ch_recv[SOCKET_BUF_MAX];
    FILE* file = fopen(file_name, "rb");
    if (!file)
    {
        s_log(LOG_ERROR, "[client] config open file error. %s.", file_name);
        return CLIENT_ERROR_SEND_NEW;
    }
    fseek(file, 0, SEEK_SET);
    long read_len = 0;
    for (long i = 0;i < *file_len;i = i + read_len)
    {
        memset(ch_send, 0, SOCKET_BUF_MAX);
        read_len = fread(ch_send, 1, SOCKET_BUF_MAX, file);
        if (send(client_socket, ch_send, read_len, 0) == SOCKET_ERROR)
        {
            s_log(LOG_ERROR, "[client] send failed: %d.", WSAGetLastError());
            return CLIENT_ERROR_SEND_NEW;
        }
    }
    fclose(file);
    memset(ch_recv, 0, SOCKET_BUF_MAX);
    long recv_len = recv(client_socket, ch_recv, SOCKET_BUF_MAX, 0);
    if (recv_len > 0)
    {
        struct json_object* parsed_json;
        parsed_json = json_tokener_parse(ch_recv);
        struct json_object* jres;
        if (json_object_object_get_ex(parsed_json, "result", &jres))
        {
            if (0 != strcmp(json_object_get_string(jres), "ok"))
            {
                s_log(LOG_ERROR, "[client] client_send_new data format error");
                json_object_put(parsed_json);
                return CLIENT_ERROR_SEND_NEW;
            }
        }
        json_object_put(parsed_json);
    }
    else
    {
        s_log(LOG_ERROR, "[client] client_send_new Recv failed: %d.", WSAGetLastError());
        return CLIENT_ERROR_SEND_NEW;
    }
    return 0;
}

int check_remove_file(const char* file_name)
{
    FILE* file = fopen(file_name, "rb");
    if (file)
    {
        fclose(file);
        return remove(file_name);
    }
    return 0;
}
