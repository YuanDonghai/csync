#include "monitor.h"


void start_monitor_entry()
{
    while (1)
    {
        clean_monitor_list();
        Sleep(1000);
    }
}

void clean_monitor_list()
{
    monitor_path* it_p = monitor_head_p;
    while (it_p)
    {
        if (it_p->next)
        {
            monitor_path* it_p_del = it_p->next;
            if (it_p_del->status == CLEAN_NEED)
            {
                it_p->next = it_p_del->next;
                free(it_p_del);
            }
            else
            {
                it_p = it_p->next;
                continue;
            }
        }
        else
        {
            break;
        }
    }
    // clean head
    it_p = monitor_head_p;
    if (monitor_head_p)
    {
        if (monitor_head_p->status == CLEAN_NEED)
        {
            monitor_head_p = monitor_head_p->next;
            free(it_p);
        }
    }
}
monitor_path* malloc_monitor_meta(const char* id, const char* name, const char* path, const char* address, int port)
{
    monitor_path* new_monitor_p = (monitor_path*)malloc(sizeof(monitor_path));
    memset(new_monitor_p->path, 0, MAX_PATH);
    sprintf_s(new_monitor_p->path, MAX_PATH, "%s", path);
    memset(new_monitor_p->w_path, 0, MAX_PATH);
    char_to_wchar(path, new_monitor_p->w_path);
    new_monitor_p->status = START_NEED;
    //   new_monitor_p->stop_need = FALSE;
      // new_monitor_p->clean_need = FALSE;
    new_monitor_p->instance_list = malloc_instance_meta(id, name, path, address, port);
    new_monitor_p->next = NULL;
    // start thread
    CreateThread(NULL, 0, thread_start_monitor_directory, new_monitor_p, 0, NULL);
    new_monitor_p->status = STARTED;
    return new_monitor_p;
}

DWORD WINAPI thread_start_monitor_directory(LPVOID lpParam)
{
    monitor_path* thread = (monitor_path*)lpParam;
    s_log(LOG_INFO, "starting monitor dir: %s .", thread->path);
    watch_directory(thread->w_path, thread);
    s_log(LOG_INFO, "starting monitor dir over: %s .", thread->path);
    thread->status = CLEAN_NEED;
    return 0;
}

void watch_directory(const wchar_t* directory_path, monitor_path* monitor)
{
    HANDLE hDir = CreateFileW(
        directory_path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE)
    {
        return;
    }

    char buffer[1024];
    DWORD bytesReturned;
    FILE_NOTIFY_INFORMATION* pNotify;
    int offset = 0;
    TCHAR fullPath[MAX_PATH];
    char FSecondPath[MAX_PATH];
    while (1)
    {
        if (monitor->status == STOP_NEED)
        {
            break;
        }
        if (ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            TRUE,
            FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION,
            &bytesReturned,
            NULL,
            NULL
        ))
        {
            pNotify = (FILE_NOTIFY_INFORMATION*)buffer;

            while (1)
            {
                pNotify->FileName[pNotify->FileNameLength / sizeof(WCHAR)] = 0;
                _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), directory_path, pNotify->FileName);
                instance_meta* it_i_p = monitor->instance_list;
                if (4 == pNotify->Action)
                {
                    memset(FSecondPath, 0, MAX_PATH);
                    wchar_to_char(pNotify->FileName, FSecondPath);
                    offset += pNotify->NextEntryOffset;
                    pNotify = (FILE_NOTIFY_INFORMATION*)((BYTE*)pNotify + pNotify->NextEntryOffset);
                    pNotify->FileName[pNotify->FileNameLength / sizeof(WCHAR)] = 0;
                }

                while (it_i_p)
                {
                    /*
                    if (!it_i_p->next)
                    {
                        break;
                    }
                    */
                    if (5 == pNotify->Action)
                    {
                        add_sync_task_in_queue_w(it_i_p, pNotify->Action, FSecondPath, pNotify->FileName, 0);
                    }
                    else
                    {
                        add_sync_task_in_queue_w(it_i_p, pNotify->Action, monitor->path, pNotify->FileName, check_path_type(fullPath));
                    }

                    it_i_p = it_i_p->next;
                }

                if (pNotify->NextEntryOffset == 0)
                {
                    break;
                }
                offset += pNotify->NextEntryOffset;
                pNotify = (FILE_NOTIFY_INFORMATION*)((BYTE*)pNotify + pNotify->NextEntryOffset);
            }
        }
        else
        {
            s_log(LOG_ERROR, "ReadDirectoryChangesW failed: %lu\n", GetLastError());
        }
        Sleep(1000);
    }
    CloseHandle(hDir);
}
void add_sync_task_in_queue_w(instance_meta* instance_p, DWORD action, const char* m_path, const wchar_t* fname, int type)
{
    char cfpath[MAX_PATH];
    char cfname[MAX_PATH];
    memset(cfpath, 0, MAX_PATH);
    memset(cfname, 0, MAX_PATH);

    wchar_to_char(fname, cfname);

    sprintf_s(cfpath, MAX_PATH, "%s\\%s", m_path, cfname);


    if (1 == compare_path(instance_p->path, cfpath))
    {
        if (action == 5)
        {
            sprintf_s(cfpath, MAX_PATH, "%s", m_path);
            format_path(cfpath);
            format_path(cfname);
            add_sync_task_in_queue(instance_p, action, cfpath, cfname, type);
        }
        else
        {
            format_path(cfpath);
            format_path(cfname);
            add_sync_task_in_queue(instance_p, action, cfpath, cfname, type);
        }

    }
}
void add_sync_task_in_queue(instance_meta* instance_p, DWORD action, char* fname, char* short_name, int type)
{
    format_path(fname);
    format_path(short_name);
    //s_log(LOG_DEBUG, "add_sync_task_in_queue file %s ,action %d .", fname, action);
    for (int i = 0;i < TASK_QUEUE_COUNTS;i++)
    {
        if (instance_p->task_queues[i].status == 2)
        {
            //if (instance_p->task_queues[i].action == action && instance_p->task_queues[i].type == type && 0 == strcmp(instance_p->task_queues[i].name, fname) && 0 == strcmp(instance_p->task_queues[i].short_name, short_name))
            if (instance_p->task_queues[i].action == action && 0 == strcmp(instance_p->task_queues[i].name, fname))
            {
                instance_p->task_queues[i].status = 0;
                return;
            }
        }
    }
    for (int i = 0;i < TASK_QUEUE_COUNTS;i++)
    {
        if (instance_p->task_queues[i].status < 1)
        {
            instance_p->task_queues[i].action = action;
            instance_p->task_queues[i].type = type;
            sprintf_s(instance_p->task_queues[i].name, MAX_PATH, "%s", fname);
            sprintf_s(instance_p->task_queues[i].short_name, MAX_PATH, "%s", short_name);

            instance_p->task_queues[i].status = 1;
            break;
        }

        /*
        else
        {
            if (instance_p->task_queues[i].action == action && instance_p->task_queues[i].type == type && 0 == strcmp(instance_p->task_queues[i].name, fname) && 0 == strcmp(instance_p->task_queues[i].short_name, short_name))
            {
                s_log(LOG_DEBUG,"task is same,skip");
                break;
            }
        }
        */
    }
}
void add_self_task_in_queue(instance_meta* instance_p, DWORD action, char* fname, char* short_name, int type)
{
    // s_log(LOG_DEBUG, "add_self_task_in_queue file %s ,action %d .", fname, action);
    for (int i = 0;i < TASK_QUEUE_COUNTS;i++)
    {
        if (instance_p->task_queues[i].status < 1)
        {
            instance_p->task_queues[i].action = action;
            instance_p->task_queues[i].type = type;
            sprintf_s(instance_p->task_queues[i].name, MAX_PATH, "%s", fname);
            sprintf_s(instance_p->task_queues[i].short_name, MAX_PATH, "%s", short_name);

            instance_p->task_queues[i].status = 2;
            break;
        }
    }
}

int compare_path(const char* path1, const char* path2)
{
    int same_len = 0;
    int len1 = strlen(path1);
    int len2 = strlen(path2);
    for (same_len = 0;same_len < len1 && same_len < len2;same_len++)
    {
        if (path1[same_len] != path2[same_len])
        {
            break;
        }
    }

    if (same_len < len1 && same_len < len2)
    {
        return 0;
    }

    if (same_len == len1)
    {
        if (path2[same_len] == '\\')
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    if (same_len == len2)
    {
        if (path1[same_len] == '\\')
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

instance_meta* malloc_instance_meta(const char* id, const char* name, const char* path, const char* address, int port)
{
    instance_meta* new_instance_p = (instance_meta*)malloc(sizeof(instance_meta));
    memset(new_instance_p->id, 0, INSTANCE_ID_LEN);
    sprintf_s(new_instance_p->id, INSTANCE_ID_LEN, "%s", id);
    memset(new_instance_p->name, 0, INSTANCE_NAME_LEN);
    sprintf_s(new_instance_p->name, INSTANCE_NAME_LEN, "%s", name);
    memset(new_instance_p->path, 0, MAX_PATH);
    sprintf_s(new_instance_p->path, INSTANCE_ID_LEN, "%s", path);
    memset(new_instance_p->peer_address, 0, 16);
    sprintf_s(new_instance_p->peer_address, 16, "%s", address);
    new_instance_p->peer_port = port;
    //new_instance_p->con = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    new_instance_p->time_out_status = FALSE;
    new_instance_p->need_reconnect = TRUE;
    new_instance_p->next = NULL;
    new_instance_p->manual_sync_status = 0;

    // start sync task thread
    CreateThread(NULL, 0, thread_start_sync_task, new_instance_p, 0, NULL);

    return new_instance_p;
}

DWORD WINAPI thread_start_sync_task(LPVOID lpParam)
{
    instance_meta* instance_p = (instance_meta*)lpParam;
    s_log(LOG_INFO, "starting sync thread for instance : %s .", instance_p->id);
    int i = 0;
    while (1)
    {
        /*
        // start sync the instance all file
        if (instance_p->manual_sync_status == 0)
        {
            s_log(LOG_INFO, "starting sync instance:%s dir.", instance_p->id);
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con));
                instance_p->need_reconnect = FALSE;
                client_sync_path(instance_p->con, instance_p->id);
            }
            TCHAR full_path[MAX_PATH];
            char_to_wchar(instance_p->path, full_path);
            client_sync_dir(instance_p->con, full_path, L"");
            instance_p->manual_sync_status = 1;
            s_log(LOG_INFO, "sync instance:%s dir over.", instance_p->id);
            continue;
        }
        */
        for (i = 0;i < TASK_QUEUE_COUNTS;i++)
        {
            if (instance_p->task_queues[i].status == 0)
            {
                break;
            }
            if (instance_p->task_queues[i].status == 1)
            {                
                if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
                {
                    client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con));
                    instance_p->need_reconnect = FALSE;
                    client_sync_path(instance_p->con, instance_p->id);
                    //client_sync_file(instance_p->con, instance_p->task_queues[i].name, instance_p->task_queues[i].short_name);
                }
                else
                {
                    //client_sync_file(instance_p->con, instance_p->task_queues[i].name, instance_p->task_queues[i].short_name);
                }
                switch (instance_p->task_queues[i].action)
                {
                case 1:// create
                    if (instance_p->task_queues[i].type == 0)//file
                    {
                        if (0 < monitor_get_file_length(instance_p->task_queues[i].name))
                        {
                            client_sync_file(instance_p->con, instance_p->task_queues[i].name, instance_p->task_queues[i].short_name);
                        }
                        else
                        {
                            client_create_file(instance_p->con, instance_p->task_queues[i].short_name);
                        }
                    }
                    if (instance_p->task_queues[i].type == 1)//dir
                    {
                        client_create_dir(instance_p->con, instance_p->task_queues[i].short_name);
                    }
                    break;
                case 2://delete
                    client_delete_file(instance_p->con, instance_p->task_queues[i].short_name);
                    break;
                case 3://write
                    if (0 < monitor_get_file_length(instance_p->task_queues[i].name))
                    {
                        client_sync_file(instance_p->con, instance_p->task_queues[i].name, instance_p->task_queues[i].short_name);
                    }
                    else
                    {
                        client_create_file(instance_p->con, instance_p->task_queues[i].short_name);
                    }
                    break;
                case 5://rename
                    client_rename_file(instance_p->con, instance_p->task_queues[i].name, instance_p->task_queues[i].short_name);
                    break;
                default:
                    break;
                }
                instance_p->task_queues[i].status = 0;
                instance_p->timestap = time(NULL);
                instance_p->time_out_status = TRUE;
            }
        }

        if (instance_p->time_out_status)
        {
            time_t cur_time = time(NULL);
            double difference = difftime(cur_time, instance_p->timestap);
            if (difference > INSTANCE_SOCKET_TIMEOUT_SEC)
            {
                s_log(LOG_INFO, "connection close for instance: %s", instance_p->id);
                closesocket(instance_p->con);
                instance_p->time_out_status = FALSE;
                instance_p->need_reconnect = TRUE;
            }
        }
        Sleep(1000);
    }

    return 0;
}

void add_instance_in_monitor(const char* id, const char* name, const char* path, const char* address, int port)
{
    if (!monitor_head_p)
    {
        monitor_head_p = malloc_monitor_meta(id, name, path, address, port);
    }
    else
    {
        monitor_path* it_p = monitor_head_p;
        monitor_path* new_p = NULL;
        BOOL new_m = TRUE;
        int res = 0;
        while (it_p)
        {
            // compare path
            res = compare_path(it_p->path, path);
            if (res == 0)
            {
                if (!it_p->next)
                {
                    break;
                }
                it_p = it_p->next;
                continue;
            }
            new_m = FALSE;
            if (res == 2)
            {
                if (!new_p)
                {
                    new_p = malloc_monitor_meta(id, name, path, address, port);
                }
                instance_meta* it_i_p = new_p->instance_list;
                while (it_i_p)
                {
                    if (!it_i_p->next)
                    {
                        break;
                    }
                    it_i_p = it_i_p->next;
                }
                it_i_p->next = it_p->instance_list;

                it_p->status = STOP_NEED;
                if (!it_p->next)
                {
                    break;
                }
                it_p = it_p->next;
                continue;
            }
            if (res == 1)
            {
                instance_meta* it_i_p = it_p->instance_list;
                while (it_i_p)
                {
                    if (!it_i_p->next)
                    {
                        break;
                    }
                    it_i_p = it_i_p->next;
                }
                it_i_p->next = malloc_instance_meta(id, name, path, address, port);
                break;
            }
        }
        if (!it_p->next && new_m)
        {
            it_p->next = malloc_monitor_meta(id, name, path, address, port);
        }
        if (new_p && !it_p->next)
        {
            it_p->next = new_p;
        }
    }

}

int check_path_type(const char* path)
{
    DWORD attributes = GetFileAttributes(path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return -1;
    }
    else
    {
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}


long monitor_get_file_length(char* fname)
{
    FILE* file = fopen(fname, "rb");
    if (!file)
    {
        s_log(LOG_DEBUG, "config open file error. %s.", fname);
        return -1;
    }

    fseek(file, 0, SEEK_END);

    long len = ftell(file);
    fclose(file);
    return len;
}

instance_meta* search_instance_p(const char* instance_id)
{
    monitor_path* it_p = monitor_head_p;
    while (it_p)
    {
        instance_meta* it_i_p = it_p->instance_list;
        while (it_i_p)
        {
            if (0 == strcmp(it_i_p->id, instance_id))
            {
                return it_i_p;
            }
            it_i_p = it_i_p->next;
        }
        it_p = it_p->next;
    }
    return it_p;
}
