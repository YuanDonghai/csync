#include "protocol.h"

#if defined(_WIN32) || defined(_WIN64)
int create_dir(const char* dirname)
{
    DWORD attributes = GetFileAttributesA(dirname);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        if (CreateDirectoryA(dirname, NULL))
        {
            return 0;
        }
        else
        {
            s_log(LOG_ERROR, "Failed to create folder: %s. Error: %lu\n", dirname, GetLastError());
            return -1;
        }
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return 0;
    }
    s_log(LOG_ERROR, "Failed to create folder: %s. path exist", dirname);
    return -1;
}
int delete_directory(LPCTSTR dir_path)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR searchPath[FILE_PATH_MAX_LEN];
    TCHAR fullPath[FILE_PATH_MAX_LEN];

    _stprintf_s(searchPath, FILE_PATH_MAX_LEN, _T("%s\\*"), dir_path);
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        s_log(LOG_ERROR, "FindFirstFile failed with error % lu\n", GetLastError());
        return -1;
    }

    do
    {
        const TCHAR* name = findFileData.cFileName;
        if (_tcscmp(name, _T(".")) == 0 || _tcscmp(name, _T("..")) == 0)
        {
            continue;
        }

        _stprintf_s(fullPath, FILE_PATH_MAX_LEN, _T("%s\\%s"), dir_path, name);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            delete_directory(fullPath);
        }
        else
        {
            if (!DeleteFile(fullPath))
            {
                s_log(LOG_ERROR, "DeleteFile failed with error %lu for file: %s\n", GetLastError(), fullPath);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        s_log(LOG_ERROR, "Error: FindNextFile failed with error %lu\n", dwError);
    }

    FindClose(hFind);
    if (!RemoveDirectory(dir_path))
    {
        s_log(LOG_ERROR, "RemoveDirectory failed with error %lu for directory: %s\n", GetLastError(), dir_path);
        return -1;
    }

    return 0;
}

int remove_dir(const char* dirname)
{
    wchar_t w_dirname[FILE_PATH_MAX_LEN];
    char_to_wchar(dirname, w_dirname, FILE_PATH_MAX_LEN, CP_ACP);
    if (0 == check_path_type(w_dirname))
    {
        return !DeleteFile(w_dirname);
    }
    else
    {
        return delete_directory(w_dirname);
    }

}
#elif defined(__linux__)
int create_dir(const char* dirname)
{
    if (mkdir(dirname, 0755) == -1)
    {
        return -1;
    }
    return 0;
}
int remove_dir(const char* dirname)
{
    return remove(dirname);
}
#else
// others
#endif

int push_stream_to_data(char* data, unsigned long len, sync_protocol* protocol)
{
    int res = 0;
    int stable_status = 1;
    if (len < 4096)
    {
        data[len] = 0;
    }
    switch (protocol->status)
    {
    case READY_BASE:
        res = trans_status_on_path(data, len, protocol);
        stable_status = 0;
        break;
    case READY_SYNC:
        res = trans_status_on_ready(data, len, protocol);
        stable_status = 0;
        break;
    case SERVER_ACK_SIG:
        res = trans_status_on_ack_sig(data, len, protocol);
        stable_status = 0;
        break;
    case SERVER_SEND_SIG:
        res = trans_status_on_req_send_del(data, len, protocol);
        stable_status = 0;
        break;
    case SERVER_RECV_DEL:
    case CLIENT_SENDING_DEL:
        res = trans_status_on_ack_send_del(data, len, protocol);
        if (protocol->status == SERVER_RECV_END)
        {
            stable_status = 0;
        }
        break;
    case SERVER_RECV_END:
        res = trans_status_on_ack_del(data, len, protocol);

        break;
    case SERVER_RECVING_NEW:
        res = trans_status_on_recv_new(data, len, protocol);
        if (protocol->status == SERVER_RECV_NEW_END)
        {
            stable_status = 0;
        }
        break;
    default:
        break;
    }
    // return stable_status;
    /*error*/
    if (res == 0)
    {
    }
    else
    {
        set_protocol_status_error(protocol);
        stable_status = 1;
    }
    return stable_status;
}
void post_update_status(sync_protocol* protocol)
{
    switch (protocol->status)
    {
    case PATH_SYNC:
        protocol->status = READY_SYNC;
        break;
    case CLIENT_REQ_SIG:
        protocol->status = SERVER_ACK_SIG;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case CLIENT_RECV_SIG:
        protocol->status = SERVER_SEND_SIG;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case CLIENT_SEND_DEL:
        if (protocol->will_recv_data_len > 9)
        {
            protocol->status = SERVER_RECV_DEL;
        }
        else
        {
            protocol->status = READY_SYNC;
        }
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case SERVER_PATCHED:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case CLIENT_SEND_NEW:
        protocol->status = SERVER_RECVING_NEW;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case SERVER_RECV_NEW_END:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case ERROR_STS:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case SERVER_ACK_DIR:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case SERVER_ACK_FILE:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case SERVER_DEL_FILE:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case SERVER_RENAME_FILE:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case TIME_SYNC:
        protocol->status = READY_BASE;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case NOTICE_SYNC_ALL:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    case CLIENT_CHECK_FILE_E:
        protocol->status = READY_SYNC;
        protocol->data_len = 0;
        free(protocol->data);
        break;
    default:
        break;
    }
}

void reset_status(sync_protocol* protocol)
{
    protocol->status = READY_SYNC;
    if (protocol->data_len > 0)
    {
        protocol->data_len = 0;
        free(protocol->data);
    }
    if (protocol->next_data_len > 0)
    {
        protocol->next_data_len = 0;
        free(protocol->next_data);
    }
}

void set_protocol_status_error(sync_protocol* protocol)
{
    protocol->data_len = (long)strlen("{\"result\":\"error\"}");
    protocol->data = (char*)malloc(sizeof(char) * protocol->data_len);
    memcpy(protocol->data, "{\"result\":\"error\"}", protocol->data_len);
    protocol->status = ERROR_STS;
}

void set_protocol_status_ok(enum sync_status status, sync_protocol* protocol)
{
    protocol->status = status;
    protocol->data_len = (long)strlen("{\"result\":\"ok\"}");
    protocol->data = (char*)malloc(sizeof(char) * protocol->data_len);
    memcpy(protocol->data, "{\"result\":\"ok\"}", protocol->data_len);
}

int trans_status_on_path(char* data, unsigned long len, sync_protocol* protocol)
{
    struct json_object* parsed_json;
    parsed_json = json_tokener_parse(data);
    if (!parsed_json)
    {
        s_log(LOG_ERROR, "Error parsing JSON.");
        return SYNC_STATUS_ERROR_FORMAT;
    }
    struct json_object* jtype;
    struct json_object* jvalue;

    if (json_object_object_get_ex(parsed_json, "type", &jtype))
    {
        switch (json_object_get_int(jtype))
        {
        case PATH_SYNC:
            if (json_object_object_get_ex(parsed_json, "id", &jvalue))
            {
                if (0 == update_instance(json_object_get_string(jvalue), protocol))
                {
                    s_log(LOG_INFO, "[server] set sync intance, id=%s path=%s.", protocol->instance_id, protocol->instance_path);
                    set_protocol_status_ok(PATH_SYNC, protocol);
                }
                else
                {
                    return SYNC_STATUS_ERROR_FORMAT;
                }
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }
            break;
        case TIME_SYNC:
            check_locals_time(protocol);
            break;
        default:
            return SYNC_STATUS_ERROR_ERROR_STEP;
            break;
        }
    }
    return 0;
}

int trans_status_on_ready(char* data, unsigned long len, sync_protocol* protocol)
{
    struct json_object* parsed_json;
    parsed_json = json_tokener_parse(data);
    if (!parsed_json)
    {
        s_log(LOG_DEBUG, "[server] Error parsing JSON.");
        return SYNC_STATUS_ERROR_FORMAT;
    }

    struct json_object* jtype;
    char fname1[FILE_NAME_MAX_LENGTH];
    char fname2[FILE_NAME_MAX_LENGTH];
    if (json_object_object_get_ex(parsed_json, "type", &jtype))
    {
        struct json_object* jvalue;
        switch (json_object_get_int(jtype))
        {
        case CLIENT_CHECK_FILE_E:
            if (json_object_object_get_ex(parsed_json, "file", &jvalue))
            {
                memset(protocol->file_name, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(protocol->file_name, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
                if (json_object_object_get_ex(parsed_json, "time", &jvalue))
                {
                    protocol->file_time = json_object_get_int64(jvalue);
                }
                if (0 == file_exist(protocol->file_name))
                {
                    protocol->instance_p->task_push = 1;
                    if (0 == touch_file(protocol->file_name))
                    {
                        update_file_time(protocol->file_name, protocol->file_time);
                        set_protocol_status_ok(CLIENT_CHECK_FILE_E, protocol);
                        add_self_task_in_queue(protocol->instance_p, 1, protocol->file_name, "", 0);
                    }
                    else
                    {
                        return SYNC_STATUS_ERROR_FORMAT;
                    }
                    protocol->instance_p->task_push = 0;
                }
                else
                {
                    long local_file_time = get_file_timestap(protocol->file_name);
                    //s_log(LOG_DEBUG, "[time] %ld %ld. %ld", file_time, local_file_time, file_time - local_file_time);
                    if (local_file_time < protocol->file_time)
                    {
                        protocol->instance_p->task_push = 1;
                        if (0 == touch_file(protocol->file_name))
                        {
                            update_file_time(protocol->file_name, protocol->file_time);
                            set_protocol_status_ok(CLIENT_CHECK_FILE_E, protocol);
                            add_self_task_in_queue(protocol->instance_p, 1, protocol->file_name, "", 0);
                        }
                        else
                        {
                            return SYNC_STATUS_ERROR_FORMAT;
                        }
                        protocol->instance_p->task_push = 0;
                    }
                    else
                    {
                        set_protocol_status_ok(CLIENT_CHECK_FILE_E, protocol);
                    }

                }
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }
            break;
        case CLIENT_REQ_SIG:
            if (json_object_object_get_ex(parsed_json, "file", &jvalue))
            {

                memset(protocol->file_name, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(protocol->file_name, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
                s_log(LOG_INFO, "[server] client %s syncing file [%s] to local: [%s].", protocol->client_address, trans_char_to_local(json_object_get_string(jvalue)), protocol->file_name);
                if (json_object_object_get_ex(parsed_json, "time", &jvalue))
                {
                    protocol->file_time = json_object_get_int64(jvalue);
                }
                if (0 == file_exist(protocol->file_name))
                {
                    char respon_string[RESP_DATA_MAX_LENGTH];
                    memset(respon_string, 0, RESP_DATA_MAX_LENGTH);
                    sprintf_s(respon_string, RESP_DATA_MAX_LENGTH, "{\"type\": %d,\"length\": 0,\"checksum\":\"\"}", SERVER_ACK_SIG);
                    protocol->data = (char*)malloc(sizeof(char) * strlen(respon_string));
                    protocol->data_len = strlen(respon_string);
                    memcpy(protocol->data, respon_string, strlen(respon_string));
                    protocol->next_data_len = 0;
                    protocol->status = CLIENT_REQ_SIG;
                    s_log(LOG_DEBUG, "[server] file %s is not existed.", protocol->file_name);
                    // add_self_task_in_queue(protocol->instance_p, 1, dir_name, json_object_get_string(jvalue),1);
                }
                else
                {
                    char respon_string[RESP_DATA_MAX_LENGTH];
                    memset(respon_string, 0, RESP_DATA_MAX_LENGTH);

                    long file_time = 0;
                    long local_file_time = 0;
                    if (json_object_object_get_ex(parsed_json, "time", &jvalue))
                    {
                        file_time = json_object_get_int64(jvalue);
                        local_file_time = get_file_timestap(protocol->file_name);
                        //s_log(LOG_DEBUG, "[time] %ld %ld. %ld", file_time, local_file_time, file_time - local_file_time);
                        if (local_file_time >= file_time)
                        {
                            sprintf_s(respon_string, 4096, "{\"type\": %d,\"length\":-1,\"checksum\":\"\"}", SERVER_ACK_SIG);
                            protocol->status = ERROR_STS;
                            s_log(LOG_DEBUG, "[server] file %s is newer than client", protocol->file_name);
                        }
                        else
                        {
                            rdiff_sig(protocol->file_name, protocol->sig_name);
                            long sig_f_len = get_file_length_md5(protocol->sig_name, protocol->will_recv_checksum);
                            sprintf_s(respon_string, 4096, "{\"type\": %d,\"length\":%ld,\"checksum\":\"%s\"}", SERVER_ACK_SIG, sig_f_len, protocol->will_recv_checksum);
                            protocol->next_data_len = sig_f_len;
                            protocol->next_data = (char*)malloc(sizeof(char) * protocol->next_data_len);

                            read_file_to_buff(protocol->sig_name, protocol->next_data, protocol->next_data_len);
                            int res = remove(protocol->sig_name);
                            protocol->status = CLIENT_REQ_SIG;
                            s_log(LOG_DEBUG, "[server] file %s signature length: %ld.", protocol->file_name, sig_f_len);
                        }
                    }

                    protocol->data = (char*)malloc(sizeof(char) * strlen(respon_string));
                    protocol->data_len = strlen(respon_string);
                    memcpy(protocol->data, respon_string, strlen(respon_string));
                }
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }
            break;
        case CLIENT_REQ_DIR:
            if (json_object_object_get_ex(parsed_json, "dir", &jvalue))
            {

                s_log(LOG_INFO, "[server] client [%s] syncing dir [%s] .", protocol->client_address, json_object_get_string(jvalue));
                char dir_name[FILE_NAME_MAX_LENGTH];
                memset(dir_name, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(dir_name, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
                s_log(LOG_DEBUG, "[server] create dir %s .", dir_name);
                protocol->instance_p->task_push = 1;
                if (0 == create_dir(dir_name))
                {
                    set_protocol_status_ok(SERVER_ACK_DIR, protocol);
                    // push self action
                    add_self_task_in_queue(protocol->instance_p, 1, dir_name, json_object_get_string(jvalue), 1);
                }
                else
                {
                    return SYNC_STATUS_ERROR_FORMAT;
                }
                protocol->instance_p->task_push = 0;
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }

            break;
        case CLIENT_REQ_FILE:
            if (json_object_object_get_ex(parsed_json, "file", &jvalue))
            {
                char dir_name[FILE_NAME_MAX_LENGTH];
                memset(dir_name, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(dir_name, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
                s_log(LOG_INFO, "[server] client [%s] syncing file [%s] .", protocol->client_address, dir_name);
                s_log(LOG_DEBUG, "[server] create file %s .", dir_name);
                protocol->instance_p->task_push = 1;
                if (0 == touch_file(dir_name))
                {
                    update_file_time(dir_name, protocol->file_time);
                    set_protocol_status_ok(SERVER_ACK_FILE, protocol);
                    add_self_task_in_queue(protocol->instance_p, 1, dir_name, json_object_get_string(jvalue), 0);
                }
                else
                {
                    return SYNC_STATUS_ERROR_FORMAT;
                }
                protocol->instance_p->task_push = 0;
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }

            break;
        case CLIENT_RENAME_FILE:
            if (json_object_object_get_ex(parsed_json, "file1", &jvalue))
            {
                memset(fname1, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(fname1, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }
            if (json_object_object_get_ex(parsed_json, "file2", &jvalue))
            {
                memset(fname2, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(fname2, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }
            s_log(LOG_INFO, "client %s rename file %s to %s .", protocol->client_address, fname2, fname1);
            s_log(LOG_DEBUG, "[server] rename file %s %s .", fname2, fname1);
            protocol->instance_p->task_push = 1;
            rename(fname2, fname1);
            set_protocol_status_ok(SERVER_RENAME_FILE, protocol);
            add_self_task_in_queue(protocol->instance_p, 5, fname1, fname2, 0);
            protocol->instance_p->task_push = 0;
            break;
        case CLIENT_DEL_FILE:
            if (json_object_object_get_ex(parsed_json, "file", &jvalue))
            {
                char dir_name[FILE_NAME_MAX_LENGTH];
                memset(dir_name, 0, FILE_NAME_MAX_LENGTH);
                sprintf_s(dir_name, FILE_NAME_MAX_LENGTH, "%s%s%s", protocol->instance_path, PATH_ADD_STRING, trans_char_to_local(json_object_get_string(jvalue)));
                s_log(LOG_INFO, "[server] client %s delete file %s.", protocol->client_address, dir_name);
                protocol->instance_p->task_push = 1;
                if (0 == remove_dir(dir_name))
                {
                    set_protocol_status_ok(SERVER_DEL_FILE, protocol);
                    add_self_task_in_queue(protocol->instance_p, 2, dir_name, json_object_get_string(jvalue), 0);
                }
                else
                {
                    return SYNC_STATUS_ERROR_FORMAT;
                }
                protocol->instance_p->task_push = 0;
            }
            else
            {
                return SYNC_STATUS_ERROR_FORMAT;
            }

            break;
        case NOTICE_SYNC_ALL:
            s_log(LOG_INFO, "[server] peer %s will sync all.", protocol->client_address);
            if (protocol->instance_p->task_queues[0].status <= 0)
            {
                time(&protocol->instance_p->manual_sync_time);
                char respon_string[RESP_DATA_MAX_LENGTH];
                memset(respon_string, 0, RESP_DATA_MAX_LENGTH);
                sprintf_s(respon_string, RESP_DATA_MAX_LENGTH, "{\"type\": %d,\"result\": 1}", NOTICE_SYNC_ALL);
                protocol->data = (char*)malloc(sizeof(char) * strlen(respon_string));
                protocol->data_len = strlen(respon_string);
                memcpy(protocol->data, respon_string, strlen(respon_string));
                protocol->instance_p->manual_sync_status = 1;
            }
            else
            {
                char respon_string[RESP_DATA_MAX_LENGTH];
                memset(respon_string, 0, RESP_DATA_MAX_LENGTH);
                sprintf_s(respon_string, RESP_DATA_MAX_LENGTH, "{\"type\": %d,\"result\": 0}", NOTICE_SYNC_ALL);
                protocol->data = (char*)malloc(sizeof(char) * strlen(respon_string));
                protocol->data_len = strlen(respon_string);
                memcpy(protocol->data, respon_string, strlen(respon_string));
            }
            protocol->status = NOTICE_SYNC_ALL;
            break;
        default:
            return SYNC_STATUS_ERROR_FORMAT;
            break;
        }
    }
    else
    {
        return SYNC_STATUS_ERROR_FORMAT;
    }
    json_object_put(parsed_json);
    return 0;
}

int trans_status_on_ack_sig(char* data, unsigned long len, sync_protocol* protocol)
{
    struct json_object* parsed_json;
    parsed_json = json_tokener_parse(data);
    if (!parsed_json)
    {
        s_log(LOG_DEBUG, "[server] Error parsing JSON.");
        return SYNC_STATUS_ERROR_FORMAT;
    }

    struct json_object* jtype;
    struct json_object* jvalue;
    if (json_object_object_get_ex(parsed_json, "type", &jtype))
    {
        switch (json_object_get_int(jtype))
        {
        case CLIENT_RECV_SIG:
            s_log(LOG_DEBUG, "[server] client get signature. sig file length %d .", protocol->next_data_len);
            protocol->data = (char*)malloc(sizeof(char) * protocol->next_data_len);
            protocol->data_len = protocol->next_data_len;
            memcpy(protocol->data, protocol->next_data, protocol->next_data_len);
            protocol->next_data_len = 0;
            free(protocol->next_data);
            protocol->status = CLIENT_RECV_SIG;
            break;
        case CLIENT_SEND_NEW:

            if (json_object_object_get_ex(parsed_json, "length", &jvalue))
            {
                protocol->will_recv_data_len = json_object_get_int(jvalue);
                protocol->data_recv_data_len = protocol->will_recv_data_len;
            }
            if (json_object_object_get_ex(parsed_json, "checksum", &jvalue))
            {
                memset(protocol->will_recv_checksum, 0, CHECK_SUM_LEN);
                int m_len = strlen(json_object_get_string(jvalue));
                memcpy(protocol->will_recv_checksum, json_object_get_string(jvalue), m_len);
            }
            s_log(LOG_DEBUG, "[server] client send file.file length %d .", protocol->will_recv_data_len);
            set_protocol_status_ok(CLIENT_SEND_NEW, protocol);
            protocol->instance_p->task_push = 1;
            add_self_task_in_queue(protocol->instance_p, 1, protocol->file_name, "", 0);
            protocol->instance_p->task_push = 0;
            break;
        default:
            return SYNC_STATUS_ERROR_FORMAT;
            break;
        }
    }
    else
    {
        return SYNC_STATUS_ERROR_FORMAT;
    }
    json_object_put(parsed_json);
    return 0;
}

int trans_status_on_req_send_del(char* data, unsigned long len, sync_protocol* protocol)
{
    struct json_object* parsed_json;
    parsed_json = json_tokener_parse(data);
    if (!parsed_json)
    {
        s_log(LOG_DEBUG, "[server] Error parsing JSON.");
        return SYNC_STATUS_ERROR_FORMAT;
    }

    struct json_object* jtype;
    if (json_object_object_get_ex(parsed_json, "type", &jtype))
    {
        if (CLIENT_SEND_DEL == json_object_get_int(jtype))
        {
            struct json_object* jvalue;
            if (json_object_object_get_ex(parsed_json, "length", &jvalue))
            {
                protocol->will_recv_data_len = json_object_get_int(jvalue);
            }
            if (json_object_object_get_ex(parsed_json, "checksum", &jvalue))
            {
                memset(protocol->will_recv_checksum, 0, CHECK_SUM_LEN);
                int m_len = strlen(json_object_get_string(jvalue));
                memcpy(protocol->will_recv_checksum, json_object_get_string(jvalue), m_len);
            }
            s_log(LOG_DEBUG, "[server] client request send delta file. file length: %ld .", protocol->will_recv_data_len);
            set_protocol_status_ok(CLIENT_SEND_DEL, protocol);
        }
        else
        {
            return SYNC_STATUS_ERROR_FORMAT;
        }
    }
    else
    {
        return SYNC_STATUS_ERROR_FORMAT;
    }
    json_object_put(parsed_json);
    return 0;
}

int trans_status_on_ack_send_del(char* data, unsigned long len, sync_protocol* protocol)
{
    if (protocol->big_cache_malloc == 0)
    {
        protocol->big_cache = (char*)malloc(BIG_CACHE_SIZE * sizeof(char));
        protocol->big_cache_counts = 0;
        protocol->big_cache_malloc = 1;
    }

    if ((protocol->big_cache_counts + len > BIG_CACHE_SIZE))
    {
        FILE* file = fopen(protocol->delta_name, "ab");
        if (!file)
        {
            s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->file_name);
            return SYNC_STATUS_ERROR_SERVER_RECVING_NEW;
        }
        fwrite(protocol->big_cache, protocol->big_cache_counts, 1, file);
        protocol->big_cache_counts = 0;
        fclose(file);
        s_log(LOG_DEBUG, "[server] recv delta left %d k.", protocol->will_recv_data_len / 1024);
    }
    memcpy(&protocol->big_cache[protocol->big_cache_counts], data, len);
    protocol->big_cache_counts += len;
    protocol->will_recv_data_len -= len;
    if (protocol->will_recv_data_len <= 0)
    {
        FILE* file = fopen(protocol->delta_name, "ab");
        if (!file)
        {
            s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->file_name);
            return SYNC_STATUS_ERROR_SERVER_RECVING_NEW;
        }
        fwrite(protocol->big_cache, protocol->big_cache_counts, 1, file);
        protocol->big_cache_counts = 0;
        fclose(file);
        s_log(LOG_DEBUG, "[server] recv delta left %d k.", protocol->will_recv_data_len / 1024);
    }

    /*
    if (protocol->status == SERVER_RECV_DEL)
    {
        FILE* file = fopen(protocol->delta_name, "wb");
        if (!file)
        {
            s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->delta_name);
            return SYNC_STATUS_ERROR_SERVER_RECV_DEL;
        }
        fwrite(data, len, 1, file);
        fclose(file);
        protocol->status = CLIENT_SENDING_DEL;
        //  add_self_task_in_queue(protocol->instance_p, 1, protocol->delta_name, "", 0);
        //  add_self_task_in_queue(protocol->instance_p, 3, protocol->delta_name, "", 0);
    }
    else
    {
        FILE* file = fopen(protocol->delta_name, "ab");
        if (!file)
        {
            s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->delta_name);
            return SYNC_STATUS_ERROR_SERVER_RECV_DEL;
        }
        fwrite(data, len, 1, file);
        fclose(file);
        //   add_self_task_in_queue(protocol->instance_p, 3, protocol->delta_name, "", 0);
    }

    protocol->will_recv_data_len -= len;
    //s_log(LOG_DEBUG, "[server] client send data %ld Bytes left.", protocol->will_recv_data_len);
    */
    if (protocol->will_recv_data_len <= 0)
    {
        if (0 != check_file_with_md5(protocol->delta_name, protocol->will_recv_checksum))
        {
            protocol->status = ERROR_STS;
            return SYNC_STATUS_ERROR_SERVER_RECV_DEL;
        }
        protocol->status = SERVER_RECV_END;
    }
    return 0;
}

int trans_status_on_ack_del(char* data, unsigned long len, sync_protocol* protocol)
{
    char ch_swap_name[FILE_NAME_MAX_LENGTH];
    memset(ch_swap_name, 0, FILE_NAME_MAX_LENGTH);
    sprintf_s(ch_swap_name, FILE_NAME_MAX_LENGTH, "%s_swap", protocol->file_name);
    s_log(LOG_DEBUG, "[server] patch file to %s.", protocol->file_name);
    rs_result res = rdiff_patch(protocol->file_name, protocol->delta_name, protocol->patch_name);

    remove(protocol->delta_name);
    // old pos

    remove(protocol->file_name);
    int ress = rename(protocol->patch_name, protocol->file_name);
    update_file_time(protocol->file_name, protocol->file_time);
    remove(protocol->patch_name);
    set_protocol_status_ok(SERVER_PATCHED, protocol);
    protocol->instance_p->task_push = 1;
    add_self_task_in_queue(protocol->instance_p, 2, protocol->file_name, "", 0);
#if defined(_WIN32) || defined(_WIN64)
    add_self_task_in_queue(protocol->instance_p, 1, protocol->file_name, "", 0);
#elif defined(__linux__)
    add_self_task_in_queue(protocol->instance_p, 5, protocol->file_name, "", 0);
#else
    // others
#endif
    protocol->instance_p->task_push = 0;
    return 0;
}

int trans_status_on_recv_new(char* data, unsigned long len, sync_protocol* protocol)
{
    protocol->instance_p->task_push = 1;
    if (protocol->big_cache_malloc == 0)
    {
        protocol->big_cache = (char*)malloc(BIG_CACHE_SIZE * sizeof(char));
        protocol->big_cache_counts = 0;
        protocol->big_cache_malloc = 1;
    }

    if ((protocol->big_cache_counts + len > BIG_CACHE_SIZE))
    {
        FILE* file = fopen(protocol->file_name, "ab");
        if (!file)
        {
            s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->file_name);
            return SYNC_STATUS_ERROR_SERVER_RECVING_NEW;
        }
        fwrite(protocol->big_cache, protocol->big_cache_counts, 1, file);
        protocol->big_cache_counts = 0;
        fclose(file);
        add_self_task_in_queue(protocol->instance_p, 3, protocol->file_name, "", 0);
        protocol->instance_p->task_push = 0;
        s_log(LOG_DEBUG, "[server] recv new left %d k.", protocol->will_recv_data_len / 1024);
    }
    memcpy(&protocol->big_cache[protocol->big_cache_counts], data, len);
    protocol->big_cache_counts += len;
    protocol->will_recv_data_len -= len;
    if (protocol->will_recv_data_len <= 0)
    {
        FILE* file = fopen(protocol->file_name, "ab");
        if (!file)
        {
            s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->file_name);
            return SYNC_STATUS_ERROR_SERVER_RECVING_NEW;
        }
        fwrite(protocol->big_cache, protocol->big_cache_counts, 1, file);
        protocol->big_cache_counts = 0;
        fclose(file);
        add_self_task_in_queue(protocol->instance_p, 3, protocol->file_name, "", 0);
        protocol->instance_p->task_push = 0;
        s_log(LOG_DEBUG, "[server] recv new left %d k.", protocol->will_recv_data_len / 1024);
    }

    /*
    FILE* file = fopen(protocol->file_name, "ab");
    if (!file)
    {
        s_log(LOG_DEBUG, "[server] config open file error. %s.", protocol->file_name);
        return SYNC_STATUS_ERROR_SERVER_RECVING_NEW;
    }
    fwrite(data, len, 1, file);
    fclose(file);
    add_self_task_in_queue(protocol->instance_p, 3, protocol->file_name, "", 0);
    protocol->instance_p->task_push = 0;
*/

// s_log(LOG_DEBUG, "[server] client send data %ld Bytes left.", protocol->will_recv_data_len);
    if (protocol->will_recv_data_len <= 0)
    {
        free(protocol->big_cache);
        protocol->big_cache_malloc = 0;
        protocol->big_cache_counts = 0;
        if (0 != check_file_with_md5(protocol->file_name, protocol->will_recv_checksum))
        {
            s_log(LOG_DEBUG, "check_file_with_md5 error.");
            protocol->status = ERROR_STS;
            return SYNC_STATUS_ERROR_SERVER_RECVING_NEW;
        }
        update_file_time(protocol->file_name, protocol->file_time);
        add_self_task_in_queue(protocol->instance_p, 3, protocol->file_name, "", 0);
        set_protocol_status_ok(SERVER_RECV_NEW_END, protocol);
    }
    return 0;
}

int update_instance(const char* instance_id, sync_protocol* protocol)
{
    char ch_instance_path[FILE_NAME_MAX_LENGTH];
    memset(protocol->instance_id, 0, INSTANCE_ID_LEN);
    memcpy(protocol->instance_id, instance_id, strlen(instance_id));

    get_instance_path(protocol->instance_id, ch_instance_path);
    s_log(LOG_DEBUG, "[server] instance id %s, path=%s", protocol->instance_id, ch_instance_path);
#if defined(_WIN32) || defined(_WIN64)
    if (0 == _access(ch_instance_path, 0))
#elif defined(__linux__)
    if (0 == access(ch_instance_path, 0))
#else
    // others
#endif

    {
        memset(protocol->instance_path, 0, FILE_NAME_MAX_LENGTH);
        sprintf_s(protocol->instance_path, FILE_NAME_MAX_LENGTH, "%s", ch_instance_path);
        protocol->instance_p = search_instance_p(instance_id);

#if defined(_WIN32) || defined(_WIN64)
        sprintf_s(protocol->cache_path, FILE_NAME_MAX_LENGTH, "cache\\%s\\", protocol->instance_p->id);
#elif defined(__linux__)
        sprintf_s(protocol->cache_path, FILE_NAME_MAX_LENGTH, "cache/%s/", protocol->instance_p->id);
#else
        // others
#endif
        if (0 == create_dir(protocol->cache_path))
        {
        }
        memset(protocol->sig_name, 0, FILE_NAME_MAX_LENGTH);
        sprintf_s(protocol->sig_name, FILE_NAME_MAX_LENGTH, "%s%s%d", protocol->cache_path, "sig", protocol->socket);
        memset(protocol->delta_name, 0, FILE_NAME_MAX_LENGTH);
        sprintf_s(protocol->delta_name, FILE_NAME_MAX_LENGTH, "%s%s%d", protocol->cache_path, "del", protocol->socket);
        memset(protocol->patch_name, 0, FILE_NAME_MAX_LENGTH);
        sprintf_s(protocol->patch_name, FILE_NAME_MAX_LENGTH, "%s%s%d", protocol->cache_path, "patch", protocol->socket);
        return 0;
    }
    s_log(LOG_ERROR, "file %s not .", ch_instance_path);
    return 1;
}

int check_locals_time(sync_protocol* protocol)
{
    time_t cur_time;
    time(&cur_time);
    char respon_string[RESP_DATA_MAX_LENGTH];
    memset(respon_string, 0, RESP_DATA_MAX_LENGTH);
    if (1 == get_time_adj_status())
    {
        sprintf_s(respon_string, 4096, "{\"type\": %d,\"time\":%ld}", TIME_SYNC, cur_time);

    }
    else
    {
        sprintf_s(respon_string, 4096, "{\"type\": %d,\"time\":%ld}", TIME_SYNC, 0);
    }
    protocol->data = (char*)malloc(sizeof(char) * strlen(respon_string));
    protocol->data_len = strlen(respon_string);
    memcpy(protocol->data, respon_string, strlen(respon_string));
    protocol->status = TIME_SYNC;
}

BOOL file_exist(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int touch_file(const char* filename)
{
    FILE* file = fopen(filename, "wb");
    if (file)
    {
        fclose(file);
        return 0;
    }
    return 1;
}

long get_file_length(char* fname)
{
    FILE* file = fopen(fname, "rb");
    if (!file)
    {
        s_log(LOG_DEBUG, "[server] config open file error. %s.", fname);
        return -1;
    }

    fseek(file, 0, SEEK_END);

    long len = ftell(file);
    fclose(file);
    return len;
}

int read_file_to_buff(char* fname, char* data, long len)
{
    FILE* file = fopen(fname, "rb");
    if (!file)
    {
        s_log(LOG_DEBUG, "[server] config open file error. %s.", fname);
        return -1;
    }

    fseek(file, 0, SEEK_SET);
    fread(data, 1, len, file);
    fclose(file);
    return 0;
}

long get_file_length_md5(const char* file_name, char* buf)
{
    FILE* fp = fopen(file_name, "rb");
    long flen = 0;
    // errno_t err = fopen_s(&fp, file_name, "rb");
    char ch_out[32];
    char ch_out_hex[64];
    memset(ch_out, 0, 32);
    md5_stream(fp, ch_out);
    flen = ftell(fp);
    fclose(fp);
    trans_ascii_to_hex(ch_out, 32, ch_out_hex);
    ch_out_hex[32] = 0;
    memcpy(buf, ch_out_hex, 33);
    return flen;
}

#if defined(_WIN32) || defined(_WIN64)
void update_file_time(const char* fname, time_t f_time)
{
    HANDLE hFile;
    FILETIME ftModify;
    ULARGE_INTEGER uli;
    uli.QuadPart = f_time * 10000000ULL + 116444736000000000ULL;
    ftModify.dwLowDateTime = uli.LowPart;
    ftModify.dwHighDateTime = uli.HighPart;

    wchar_t wfilename[FILE_PATH_MAX_LEN];
    char_to_wchar(fname, wfilename, FILE_PATH_MAX_LEN, CP_ACP);

    hFile = CreateFileW(wfilename, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (!SetFileTime(hFile, NULL, NULL, &ftModify))
    {
        CloseHandle(hFile);
        return;
    }

    CloseHandle(hFile);
}
#elif defined(__linux__)
void update_file_time(const char* fname, time_t f_time)
{
    struct timespec times[2];

    // 设置访问时间和修改时间（这里只修改修改时间）
    times[0].tv_sec = 0;  // 访问时间设为当前时间（可选）
    times[0].tv_nsec = UTIME_OMIT;

    times[1].tv_sec = f_time;  // 修改时间
    times[1].tv_nsec = 0;

    // 使用 utimensat 修改时间
    if (utimensat(AT_FDCWD, fname, times, 0) == -1) {
        return; // 失败
    }

    return; // 成功
}
#else
//others
#endif