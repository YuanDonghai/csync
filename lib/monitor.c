#include "monitor.h"


void start_monitor_entry(int is_time_adj_g)
{
    if (is_time_adj_g == 0)
    {
        s_log(LOG_DEBUG, "check time adjustment.");
        is_time_adj = get_time_adj_status();
    }
    while (1)
    {
        clean_monitor_list();
        _sleep_or_Sleep(1000);
    }
}

void clean_monitor_list()
{
    struct monitor_path* it_p = monitor_head_p;
    while (it_p)
    {
        if (it_p->next)
        {
            struct monitor_path* it_p_del = it_p->next;
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
struct monitor_path* malloc_monitor_meta(const char* id, const char* peer_id, const char* name, const char* path, const char* address, int port)
{
    struct monitor_path* new_monitor_p = (struct monitor_path*)malloc(sizeof(struct monitor_path));
    memset(new_monitor_p->path, 0, FILE_PATH_MAX_LEN);
    sprintf_s(new_monitor_p->path, FILE_PATH_MAX_LEN, "%s", path);
    memset(new_monitor_p->w_path, 0, FILE_PATH_MAX_LEN);
#if defined(_WIN32) || defined(_WIN64)   
    char_to_wchar(path, new_monitor_p->w_path, FILE_PATH_MAX_LEN, CP_ACP);
#elif defined(__linux__)
    sprintf_s(new_monitor_p->w_path, FILE_PATH_MAX_LEN, "%s", path);
    new_monitor_p->wds = (struct inotify_wd*)malloc(sizeof(struct inotify_wd));
    new_monitor_p->wds->wd = 0;
#else
    //others
#endif

    new_monitor_p->status = START_NEED;
    //   new_monitor_p->stop_need = FALSE;
      // new_monitor_p->clean_need = FALSE;
    new_monitor_p->instance_list = malloc_instance_meta(id, peer_id, name, path, address, port);
    new_monitor_p->next = NULL;
    // start thread
#if defined(_WIN32) || defined(_WIN64)
    CreateThread(NULL, 0, thread_start_monitor_directory, new_monitor_p, 0, NULL);
#elif defined(__linux__)
    pthread_t new_mon_t;
    pthread_create(&new_mon_t, NULL, thread_start_monitor_directory, new_monitor_p);
#else
//others
#endif

    new_monitor_p->status = STARTED;
    return new_monitor_p;
}
struct monitor_path* malloc_monitor_meta_s(struct instance_meta* instance_info)
{
    struct monitor_path* new_monitor_p = (struct monitor_path*)malloc(sizeof(struct monitor_path));
    memset(new_monitor_p->path, 0, FILE_PATH_MAX_LEN);
    sprintf_s(new_monitor_p->path, FILE_PATH_MAX_LEN, "%s", instance_info->path);
    memset(new_monitor_p->w_path, 0, FILE_PATH_MAX_LEN);
#if defined(_WIN32) || defined(_WIN64)
    char_to_wchar(instance_info->path, new_monitor_p->w_path, FILE_PATH_MAX_LEN, CP_ACP);
#elif defined(__linux__)
    sprintf_s(new_monitor_p->w_path, FILE_PATH_MAX_LEN, "%s", instance_info->path);
    new_monitor_p->wds = (struct inotify_wd*)malloc(sizeof(struct inotify_wd));
    new_monitor_p->wds->wd = 0;
#else
    //others
#endif

    new_monitor_p->status = START_NEED;
    //   new_monitor_p->stop_need = FALSE;
      // new_monitor_p->clean_need = FALSE;
    new_monitor_p->instance_list = malloc_instance_meta_s(instance_info);
    new_monitor_p->next = NULL;
    // start thread
#if defined(_WIN32) || defined(_WIN64)
    CreateThread(NULL, 0, thread_start_monitor_directory, new_monitor_p, 0, NULL);
#elif defined(__linux__)
    pthread_t new_mon_t;
    pthread_create(&new_mon_t, NULL, thread_start_monitor_directory, new_monitor_p);
#else
//others
#endif

    new_monitor_p->status = STARTED;
    return new_monitor_p;
}

#if defined(_WIN32) || defined(_WIN64)
DWORD WINAPI thread_start_monitor_directory(LPVOID lpParam)
{
    struct monitor_path* thread = (struct monitor_path*)lpParam;
    s_log(LOG_INFO, "starting monitor dir: %s .", thread->path);
    watch_directory(thread->w_path, thread);
    s_log(LOG_INFO, "starting monitor dir over: %s .", thread->path);
    thread->status = CLEAN_NEED;
    return 0;
}

DWORD WINAPI thread_start_sync_task(LPVOID lpParam)
{
    struct instance_meta* instance_p = (struct instance_meta*)lpParam;
    s_log(LOG_INFO, "starting sync thread for instance : %s .", instance_p->id);
    int i = 0;
    while (1)
    {
        // sync time
        if (instance_p->is_time_adj == 0)
        {
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                if (0 != client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                {
                    _sleep_or_Sleep(1000 * 10);
                    continue;
                }
                client_sync_time(instance_p->con);
                closesocket(instance_p->con);
                instance_p->is_time_adj = 1;
            }
        }

        // if not task and manual_sync_status==0, ready to notice both to sync all
        if (instance_p->manual_sync_status == 0 && instance_p->task_queues[0].status <= 0)
        {
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                if (0 != client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                {
                    _sleep_or_Sleep(1000 * 10);
                    continue;
                }
                //instance_p->need_reconnect = FALSE;
                client_sync_path(instance_p->con, instance_p->peer_id);
            }
            instance_p->manual_sync_status = client_notice_sync(instance_p->con);
            if (instance_p->manual_sync_status == 0)
            {
                continue;
            }
            closesocket(instance_p->con);
        }
        // start sync all dir
        if (instance_p->manual_sync_status == 1)
        {
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                if (0 != client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                {
                    _sleep_or_Sleep(1000 * 10);
                    continue;
                }
                //instance_p->need_reconnect = FALSE;
                client_sync_path(instance_p->con, instance_p->peer_id);
            }
            time_t cur_time;
            time(&cur_time);
            // sync dir
            s_log(LOG_INFO, "[client] starting sync instance %s all dir: %s, time:%ld.", instance_p->id, instance_p->path, cur_time);
            TCHAR full_path[FILE_PATH_MAX_LEN];
            char_to_wchar(instance_p->path, full_path, FILE_PATH_MAX_LEN, CP_ACP);
            client_sync_dir(instance_p->con, full_path, L"", cur_time, instance_p->os_type);
            instance_p->manual_sync_status = 2;
            closesocket(instance_p->con);
        }
        /*
        if (instance_p->task_push == 1)
        {
            _sleep_or_Sleep(100);
            continue;
        }
        */
        for (i = 0;i < TASK_QUEUE_COUNTS;i++)
        {
            if (instance_p->task_queues[i].status == 0)
            {
                continue;
            }
            if (instance_p->task_queues[i].status == 1)
            {
                time_t cur_time;
                time(&cur_time);
                if (FILE_CHAGNED_SECS < difftime(cur_time, instance_p->task_queues[i].timestap))
                {
                    s_log(LOG_DEBUG, "[client] client will sync file: %s", instance_p->task_queues[i].short_name);
                    if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
                    {
                        if (0 == client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                        {
                            instance_p->need_reconnect = FALSE;
                            client_sync_path(instance_p->con, instance_p->peer_id);
                        }
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
                    instance_p->time_out_status = 1;
                }
                else
                {
                    continue;
                }
            }
        }

        if (instance_p->time_out_status)
        {
            time_t cur_time = time(NULL);
            double difference = difftime(cur_time, instance_p->timestap);
            if (difference > INSTANCE_SOCKET_TIMEOUT_SEC)
            {
                s_log(LOG_INFO, "[client] connection close for instance: %s", instance_p->id);
                closesocket(instance_p->con);
                instance_p->time_out_status = 0;
                instance_p->need_reconnect = 1;
            }
        }
        Sleep(1000);
    }

    return 0;
}

void watch_directory(const wchar_t* directory_path, struct monitor_path* monitor)
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
    TCHAR fullPath[FILE_PATH_MAX_LEN];
    char FSecondPath[FILE_PATH_MAX_LEN];
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
                _stprintf_s(fullPath, FILE_PATH_MAX_LEN, _T("%s\\%s"), directory_path, pNotify->FileName);
                struct instance_meta* it_i_p = monitor->instance_list;
                if (4 == pNotify->Action)
                {
                    memset(FSecondPath, 0, FILE_PATH_MAX_LEN);
                    wchar_to_char(fullPath, FSecondPath, FILE_PATH_MAX_LEN, CP_ACP);
                    offset += pNotify->NextEntryOffset;
                    pNotify = (FILE_NOTIFY_INFORMATION*)((BYTE*)pNotify + pNotify->NextEntryOffset);
                    pNotify->FileName[pNotify->FileNameLength / sizeof(WCHAR)] = 0;
                    _stprintf_s(fullPath, FILE_PATH_MAX_LEN, _T("%s\\%s"), directory_path, pNotify->FileName);
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
                        add_sync_task_in_queue_w(it_i_p, pNotify->Action, FSecondPath, fullPath, 0);
                    }
                    else
                    {
                        add_sync_task_in_queue_w(it_i_p, pNotify->Action, monitor->path, fullPath, check_path_type(fullPath));
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
        Sleep(1);
    }
    CloseHandle(hDir);
}
void add_sync_task_in_queue_w(struct instance_meta* instance_p, int action, const char* sec_fname, const wchar_t* fname, int type)
{
    char cfname1[FILE_PATH_MAX_LEN];
    char cfname2[FILE_PATH_MAX_LEN];
    memset(cfname1, 0, FILE_PATH_MAX_LEN);
    memset(cfname2, 0, FILE_PATH_MAX_LEN);

    wchar_to_char(fname, cfname1, FILE_PATH_MAX_LEN, CP_ACP);

    if (1 == compare_path(instance_p->path, cfname1))
    {
        if (action == 5)
        {
            add_sync_task_in_queue(instance_p, action, cfname1, sec_fname, type);
        }
        else
        {
            add_sync_task_in_queue(instance_p, action, cfname1, cfname2, type);
        }

    }
}

int check_path_type(wchar_t* path)
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

int get_time_adj_status()
{
    DWORD timeAdjustment = 0;
    DWORD timeIncrement = 0;
    BOOL isTimeAdjustmentDisabled = FALSE;
    int is_time_adj_s = 0;

    if (GetSystemTimeAdjustment(&timeAdjustment, &timeIncrement, &isTimeAdjustmentDisabled))
    {
        if (!isTimeAdjustmentDisabled)
        {
            is_time_adj_s = 1;
        }
        else
        {
            is_time_adj_s = 0;
        }
    }
    else
    {
        is_time_adj_s = 0;
    }
    return is_time_adj_s;
}

long get_file_timestap(const char* fname)
{
    HANDLE hFile;
    FILETIME ftCreate, ftAccess, ftModify;
    SYSTEMTIME stUTC, stLocal;
    time_t fileTime;
    wchar_t wfilename[FILE_PATH_MAX_LEN];
    char_to_wchar(fname, wfilename, FILE_PATH_MAX_LEN, CP_ACP);
    hFile = CreateFile(wfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftModify))
    {
        CloseHandle(hFile);
        return 0;
    }

    CloseHandle(hFile);
    /*
    if (!FileTimeToSystemTime(&ftModify, &stUTC))
    {
        return 0;
    }

    if (!SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal))
    {
        return 0;
    }

    SYSTEMTIME stLocalToFT;
    if (!SystemTimeToFileTime(&stLocal, &ftModify))
    {
        return 0;
    }
    */

    ULARGE_INTEGER uli;
    uli.LowPart = ftModify.dwLowDateTime;
    uli.HighPart = ftModify.dwHighDateTime;
    fileTime = (time_t)((uli.QuadPart - 116444736000000000ULL) / 10000000ULL);

    return fileTime;
}

int instance_sync_dir(struct instance_meta* instance_p, LPCTSTR full_dir_path, LPCTSTR dir_path)
{

}

#elif defined(__linux__)
// linux
void* thread_start_monitor_directory(void* lpParam)
{
    struct monitor_path* thread = (struct monitor_path*)lpParam;
    s_log(LOG_INFO, "starting monitor dir: %s .", thread->path);
    watch_directory(thread->w_path, thread);
    s_log(LOG_INFO, "starting monitor dir over: %s .", thread->path);
    thread->status = CLEAN_NEED;
    return 0;
}
void* thread_start_sync_task(void* lpParam)
{
    struct instance_meta* instance_p = (struct instance_meta*)lpParam;
    s_log(LOG_INFO, "starting sync thread for instance : %s .", instance_p->id);
    int i = 0;
    while (1)
    {
        if (instance_p->is_time_adj == 0)
        {
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                if (0 != client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                {
                    _sleep_or_Sleep(1000 * 10);
                    continue;
                }
                client_sync_time(instance_p->con);
                close(instance_p->con);
                instance_p->is_time_adj = 1;
            }

        }

        // if not task and manual_sync_status==0, ready to notice both to sync all
        if (instance_p->manual_sync_status == 0 && instance_p->task_queues[0].status <= 0)
        {
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                if (0 != client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                {
                    _sleep_or_Sleep(1000 * 10);
                    continue;
                }
                //instance_p->need_reconnect = 0;
                client_sync_path(instance_p->con, instance_p->peer_id);

            }
            instance_p->manual_sync_status = client_notice_sync(instance_p->con);
            if (instance_p->manual_sync_status == 0)
            {
                continue;
            }
            close(instance_p->con);
        }
        // start sync all dir
        if (instance_p->manual_sync_status == 1)
        {
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                if (0 != client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con)))
                {
                    _sleep_or_Sleep(1000 * 10);
                    continue;
                }
                // instance_p->need_reconnect = 0;
                client_sync_path(instance_p->con, instance_p->peer_id);
                // start sync dir
            }
            time_t cur_time;
            time(&cur_time);
            // sync dir
            s_log(LOG_INFO, "[client] starting sync instance %s all dir: %s, time:%ld.", instance_p->id, instance_p->path, cur_time);
            client_sync_dir(instance_p->con, instance_p->path, "", cur_time, instance_p->os_type);
            instance_p->manual_sync_status = 2;
            close(instance_p->con);
        }
        /*
        // start sync the instance all file

        if (instance_p->manual_sync_status == 0)
        {
            s_log(LOG_INFO, "starting sync instance:%s dir.", instance_p->id);
            if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
            {
                client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con));
                instance_p->need_reconnect = 0;
                client_sync_path(instance_p->con, instance_p->peer_id);
            }

            client_sync_dir(instance_p->con, instance_p->path, "");
            instance_p->manual_sync_status = 1;
            s_log(LOG_INFO, "sync instance:%s dir over.", instance_p->id);
            continue;
        }
        */
        /*
        if (instance_p->task_push == 1)
        {
            _sleep_or_Sleep(100);
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
                time_t cur_time;
                time(&cur_time);
                if (FILE_CHAGNED_SECS < difftime(cur_time, instance_p->task_queues[i].timestap))
                {
                    s_log(LOG_DEBUG, "[client] client will sync file: %s", instance_p->task_queues[i].short_name);
                    if (instance_p->con == INVALID_SOCKET || instance_p->need_reconnect)
                    {
                        client_sync_connect(instance_p->peer_address, instance_p->peer_port, &(instance_p->con));
                        instance_p->need_reconnect = 0;
                        client_sync_path(instance_p->con, instance_p->peer_id);
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
                    instance_p->time_out_status = 1;
                }

            }
        }

        if (instance_p->time_out_status)
        {
            time_t cur_time = time(NULL);
            double difference = difftime(cur_time, instance_p->timestap);
            if (difference > INSTANCE_SOCKET_TIMEOUT_SEC)
            {
                s_log(LOG_INFO, "[client] connection close for instance: %s", instance_p->id);
                close(instance_p->con);
                instance_p->time_out_status = 0;
                instance_p->need_reconnect = 1;
            }
        }
        _sleep_or_Sleep(1000);
    }
    s_log(LOG_INFO, "stop sync thread for instance : %s .", instance_p->id);
    return 0;
}
void add_watch_recursive(int fd, const char* path, struct inotify_wd* inotify_wd_p)
{
    DIR* dir;
    struct dirent* entry;
    char full_path[PATH_MAX];

    // Add watch to the current directory
    int wd = inotify_add_watch(fd, path, IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

    if (wd < 0) {
        perror("inotify_add_watch");
        return;
    }
    add_inotify_wd(inotify_wd_p, wd, path);
    //show_inotify_wd(inotify_wd_p);
    // Open the directory
    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }

    // Iterate over each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the full path of the entry
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Check if the entry is a directory
        if (entry->d_type == DT_DIR) {
            // Recursively add watch to the subdirectory
            add_watch_recursive(fd, full_path, inotify_wd_p);
        }
    }

    closedir(dir);
}

void watch_directory(const char* directory_path, struct monitor_path* monitor)
{
    int fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
        return;
    }
    /*
    int wd = inotify_add_watch(fd, directory_path, IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd < 0)
    {
        perror("inotify_add_watch");
        close(fd);
        return;
    }
    */
    add_watch_recursive(fd, directory_path, monitor->wds);
    char buffer[BUF_LEN];
    while (1)
    {
        ssize_t num_read = read(fd, buffer, BUF_LEN);
        if (num_read < 0)
        {
            perror("read");
            break;
        }
        char full_path[PATH_MAX];
        char old_full_path[PATH_MAX];
        for (char* p = buffer; p < buffer + num_read; )
        {
            struct inotify_event* event = (struct inotify_event*)p;
            sprintf_s(full_path, PATH_MAX, "%s/%s", get_inotify_wd_path(monitor->wds, event->wd), event->name);

            struct instance_meta* it_i_p = monitor->instance_list;
            while (it_i_p)
            {
                if (event->mask & IN_CREATE)
                {
                    add_sync_task_in_queue_w(it_i_p, 1, monitor->path, full_path, check_path_type(full_path));
                    if (1 == check_path_type(full_path))
                    {
                        add_watch_recursive(fd, full_path, monitor->wds);
                    }
                }
                if (event->mask & IN_MODIFY)
                {
                    add_sync_task_in_queue_w(it_i_p, 3, monitor->path, full_path, check_path_type(full_path));
                }
                if (event->mask & IN_DELETE)
                {
                    add_sync_task_in_queue_w(it_i_p, 2, monitor->path, full_path, check_path_type(full_path));
                }
                if (event->mask & IN_MOVED_FROM)
                {
                    sprintf_s(old_full_path, PATH_MAX, "%s/%s", get_inotify_wd_path(monitor->wds, event->wd), event->name);
                }
                if (event->mask & IN_MOVED_TO)
                {

                    add_sync_task_in_queue_w(it_i_p, 5, old_full_path, full_path, 0);

                }
                it_i_p = it_i_p->next;
            }


            p += EVENT_SIZE + event->len;
        }
    }

    //inotify_rm_watch(fd, wd);
    close(fd);
}
void add_sync_task_in_queue_w(struct instance_meta* instance_p, int action, const char* sec_fname, const char* fname, int type)
{
    char cfname1[FILE_PATH_MAX_LEN];
    char cfname2[FILE_PATH_MAX_LEN];
    memset(cfname1, 0, FILE_PATH_MAX_LEN);
    memset(cfname2, 0, FILE_PATH_MAX_LEN);

    if (1 == compare_path(instance_p->path, fname))
    {
        if (action == 5)
        {
            add_sync_task_in_queue(instance_p, action, fname, sec_fname, type);
        }
        else
        {
            add_sync_task_in_queue(instance_p, action, fname, cfname2, type);
        }

    }
}

void add_inotify_wd(struct inotify_wd* inotify_wd_p, int wd, char* path)
{
    /*
    if (inotify_wd_p == NULL)
    {
        inotify_wd_p = (struct inotify_wd*)malloc(sizeof(struct inotify_wd));
        inotify_wd_p->wd = wd;
        sprintf_s(inotify_wd_p->path, FILE_PATH_MAX_LEN, "%s", path);
        inotify_wd_p->next = NULL;
        return;
    }
    */
    struct inotify_wd* it_p = inotify_wd_p;

    while (1)
    {
        if (!it_p)
        {
            break;
        }
        if (!it_p->next)
        {
            struct inotify_wd* new_p = (struct inotify_wd*)malloc(sizeof(struct inotify_wd));
            new_p->wd = wd;
            sprintf_s(new_p->path, FILE_PATH_MAX_LEN, "%s", path);
            new_p->next = NULL;
            it_p->next = new_p;
            break;
        }
        it_p = it_p->next;
    }
}
void show_inotify_wd(struct inotify_wd* inotify_wd_p)
{
    struct inotify_wd* it_p = inotify_wd_p;
    while (it_p)
    {
        s_log(LOG_DEBUG, "%d %s", it_p->wd, it_p->path);
        it_p = it_p->next;
    }
}
const char* get_inotify_wd_path(struct inotify_wd* inotify_wd_p, int wd)
{

    struct inotify_wd* it_p = inotify_wd_p;
    while (it_p)
    {
        if (it_p->wd == wd)
        {
            return it_p->path;
        }
        it_p = it_p->next;
    }
    return "";
}

int check_path_type(const char* path)
{
    struct stat path_stat;
    if (lstat(path, &path_stat) < 0)
    {
        return -1;
    }
    if (S_ISREG(path_stat.st_mode))
    {
        return 0;
    }
    if (S_ISDIR(path_stat.st_mode))
    {
        return 1;
    }
}

int get_time_adj_status()
{
    FILE* fp = popen("timedatectl status | grep 'synchronized: yes'", "r");
    if (fp == NULL)
    {
        return 0;
    }

    char buffer[256];
    int is_synchronized = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (strstr(buffer, "synchronized: yes"))
        {
            is_synchronized = 1;
            break;
        }
    }
    pclose(fp);
    return is_synchronized;
}

long get_file_timestap(const char* fname)
{
    struct stat file_stat;

    if (stat(fname, &file_stat) == -1)
    {

        return 0;
    }

    time_t last_modified = file_stat.st_mtime;

    return last_modified;
}
#else
//others
#endif

void add_sync_task_in_queue(struct instance_meta* instance_p, int action, char* fname1, char* fname2, int type)
{
    char cfname[FILE_PATH_MAX_LEN];
    char csname[FILE_PATH_MAX_LEN];
    format_path(fname1);
    format_path(fname2);
    //s_log(LOG_DEBUG, "add_sync_task_in_queue file %s ,action %d.", fname1, action);
    time_t cur_time;
    time(&cur_time);
    if (action == 5)
    {
        sprintf_s(cfname, FILE_PATH_MAX_LEN, "%s", &fname1[strlen(instance_p->path) + 1]);
        modify_os_file_name(instance_p->os_type, cfname);
        for (int i = 0;i < TASK_QUEUE_COUNTS;i++)
        {
            if (instance_p->task_queues[i].status == 1)
            {
                if (0 == strcmp(cfname, instance_p->task_queues[i].short_name))
                {
                    // s_log(LOG_DEBUG, "add_sync_task_in_queue file %s will rename,update other task status ,action %d.", fname1, action);
                    instance_p->task_queues[i].timestap -= FILE_CHAGNED_SECS;
                }
            }
            else
            {
                instance_p->task_queues[i].action = action;
                instance_p->task_queues[i].type = type;
                instance_p->task_queues[i].status = 1;
                sprintf_s(instance_p->task_queues[i].name, FILE_PATH_MAX_LEN, "%s", &fname1[strlen(instance_p->path) + 1]);
                sprintf_s(instance_p->task_queues[i].short_name, FILE_PATH_MAX_LEN, "%s", &fname2[strlen(instance_p->path) + 1]);
                modify_os_file_name(instance_p->os_type, instance_p->task_queues[i].name);
                modify_os_file_name(instance_p->os_type, instance_p->task_queues[i].short_name);
                time(&(instance_p->task_queues[i].timestap));
                instance_p->task_queues[i].timestap -= FILE_CHAGNED_SECS;
                break;
            }
        }
    }
    else
    {
        for (int i = 0;i < TASK_QUEUE_COUNTS;i++)
        {
            if (instance_p->task_queues[i].status == 1)
            {
                if (0 == strcmp(fname1, instance_p->task_queues[i].name))
                {
                    instance_p->task_queues[i].action = action;
                    instance_p->task_queues[i].type = type;
                    time(&(instance_p->task_queues[i].timestap));
                    break;
                }
            }
            else
            {
                instance_p->task_queues[i].action = action;
                instance_p->task_queues[i].type = type;
                sprintf_s(instance_p->task_queues[i].name, FILE_PATH_MAX_LEN, "%s", fname1);
                sprintf_s(instance_p->task_queues[i].short_name, FILE_PATH_MAX_LEN, "%s", &fname1[strlen(instance_p->path) + 1]);
                modify_os_file_name(instance_p->os_type, instance_p->task_queues[i].short_name);
                time(&(instance_p->task_queues[i].timestap));
                instance_p->task_queues[i].status = 1;
                break;
            }
        }
    }
}
void add_self_task_in_queue(struct instance_meta* instance_p, int action, char* fname, char* short_name, int type)
{
    // s_log(LOG_DEBUG, "add_self_task_in_queue file %s ,action %d .", fname, action);
    _sleep_or_Sleep(100);
    char rename_f[FILE_PATH_MAX_LEN];
    if (action == 5)
    {
        sprintf_s(rename_f, FILE_PATH_MAX_LEN, "%s", &fname[strlen(instance_p->path) + 1]);
    }
    else
    {
        sprintf_s(rename_f, FILE_PATH_MAX_LEN, "%s", fname);
    }

    for (int i = 0;i < TASK_QUEUE_COUNTS;i++)
    {
        if (instance_p->task_queues[i].status == 1)
        {
            if (0 == strcmp(instance_p->task_queues[i].name, rename_f))
            {
                instance_p->task_queues[i].status = 0;
                break;
            }
        }
        else
        {
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
        if (path2[same_len] == '\\' || path2[same_len] == '/')
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
        if (path1[same_len] == '\\' || path1[same_len] == '/')
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

struct instance_meta* malloc_instance_meta(const char* id, const char* peer_id, const char* name, const char* path, const char* address, int port)
{
    struct instance_meta* new_instance_p = (struct instance_meta*)malloc(sizeof(struct instance_meta));
    memset(new_instance_p->id, 0, INSTANCE_ID_LEN);
    sprintf_s(new_instance_p->id, INSTANCE_ID_LEN, "%s", id);
    memset(new_instance_p->peer_id, 0, INSTANCE_ID_LEN);
    sprintf_s(new_instance_p->peer_id, INSTANCE_ID_LEN, "%s", peer_id);
    memset(new_instance_p->name, 0, INSTANCE_NAME_LEN);
    sprintf_s(new_instance_p->name, INSTANCE_NAME_LEN, "%s", name);
    memset(new_instance_p->path, 0, FILE_PATH_MAX_LEN);
    sprintf_s(new_instance_p->path, INSTANCE_ID_LEN, "%s", path);
    memset(new_instance_p->peer_address, 0, 16);
    sprintf_s(new_instance_p->peer_address, 16, "%s", address);
    new_instance_p->peer_port = port;
    //new_instance_p->con = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    new_instance_p->time_out_status = 0;
    new_instance_p->need_reconnect = 1;
    new_instance_p->next = NULL;
    new_instance_p->manual_sync_status = 0;

    // start sync task thread
#if defined(_WIN32) || defined(_WIN64)
    CreateThread(NULL, 0, thread_start_sync_task, new_instance_p, 0, NULL);
#elif defined(__linux__)
    pthread_t new_ins_t;
    pthread_create(&new_ins_t, NULL, thread_start_sync_task, new_instance_p);
#else
//others
#endif

    return new_instance_p;
}
struct instance_meta* malloc_instance_meta_s(struct instance_meta* instance_info)
{
    struct instance_meta* new_instance_p = (struct instance_meta*)malloc(sizeof(struct instance_meta));
    memset(new_instance_p->id, 0, INSTANCE_ID_LEN);
    sprintf_s(new_instance_p->id, INSTANCE_ID_LEN, "%s", instance_info->id);
    memset(new_instance_p->peer_id, 0, INSTANCE_ID_LEN);
    sprintf_s(new_instance_p->peer_id, INSTANCE_ID_LEN, "%s", instance_info->peer_id);
    memset(new_instance_p->name, 0, INSTANCE_NAME_LEN);
    sprintf_s(new_instance_p->name, INSTANCE_NAME_LEN, "%s", instance_info->name);
    memset(new_instance_p->path, 0, FILE_PATH_MAX_LEN);
    sprintf_s(new_instance_p->path, INSTANCE_ID_LEN, "%s", instance_info->path);
    memset(new_instance_p->peer_address, 0, 16);
    sprintf_s(new_instance_p->peer_address, 16, "%s", instance_info->peer_address);
    new_instance_p->peer_port = instance_info->peer_port;
    new_instance_p->os_type = instance_info->os_type;
    //new_instance_p->con = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    new_instance_p->time_out_status = 0;
    new_instance_p->need_reconnect = 1;
    new_instance_p->next = NULL;
    new_instance_p->manual_sync_status = 0;
    new_instance_p->is_time_adj = is_time_adj;

    // start sync task thread
#if defined(_WIN32) || defined(_WIN64)
    CreateThread(NULL, 0, thread_start_sync_task, new_instance_p, 0, NULL);
#elif defined(__linux__)
    pthread_t new_ins_t;
    pthread_create(&new_ins_t, NULL, thread_start_sync_task, new_instance_p);
#else
//others
#endif

    return new_instance_p;
}


void add_instance_in_monitor(const char* id, const char* peer_id, const char* name, const char* path, const char* address, int port)
{
    if (!monitor_head_p)
    {
        monitor_head_p = malloc_monitor_meta(id, peer_id, name, path, address, port);
    }
    else
    {
        struct monitor_path* it_p = monitor_head_p;
        struct monitor_path* new_p = NULL;
        BOOL new_m = 1;
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
            new_m = 0;
            if (res == 2)
            {
                if (!new_p)
                {
                    new_p = malloc_monitor_meta(id, peer_id, name, path, address, port);
                }
                struct instance_meta* it_i_p = new_p->instance_list;
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
                struct instance_meta* it_i_p = it_p->instance_list;
                while (it_i_p)
                {
                    if (!it_i_p->next)
                    {
                        break;
                    }
                    it_i_p = it_i_p->next;
                }
                it_i_p->next = malloc_instance_meta(id, peer_id, name, path, address, port);
                break;
            }
        }
        if (!it_p->next && new_m)
        {
            it_p->next = malloc_monitor_meta(id, peer_id, name, path, address, port);
        }
        if (new_p && !it_p->next)
        {
            it_p->next = new_p;
        }
    }

}

void add_instance_in_monitor_s(struct instance_meta* instance_info)
{
    if (!monitor_head_p)
    {
        monitor_head_p = malloc_monitor_meta_s(instance_info);
    }
    else
    {
        struct monitor_path* it_p = monitor_head_p;
        struct monitor_path* new_p = NULL;
        BOOL new_m = 1;
        int res = 0;
        while (it_p)
        {
            // compare path
            res = compare_path(it_p->path, instance_info->path);
            if (res == 0)
            {
                if (!it_p->next)
                {
                    break;
                }
                it_p = it_p->next;
                continue;
            }
            new_m = 0;
            if (res == 2)
            {
                if (!new_p)
                {
                    new_p = malloc_monitor_meta_s(instance_info);
                }
                struct instance_meta* it_i_p = new_p->instance_list;
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
                struct instance_meta* it_i_p = it_p->instance_list;
                while (it_i_p)
                {
                    if (!it_i_p->next)
                    {
                        break;
                    }
                    it_i_p = it_i_p->next;
                }
                it_i_p->next = malloc_instance_meta_s(instance_info);
                break;
            }
        }
        if (!it_p->next && new_m)
        {
            it_p->next = malloc_monitor_meta_s(instance_info);
        }
        if (new_p && !it_p->next)
        {
            it_p->next = new_p;
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

struct instance_meta* search_instance_p(const char* instance_id)
{
    struct monitor_path* it_p = monitor_head_p;
    while (it_p)
    {
        struct instance_meta* it_i_p = it_p->instance_list;
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
    s_log(LOG_DEBUG, "search no instance p:");
    return NULL;
}


void modify_os_file_name(int os_type, char* fname)
{
    char ch[2] = "\\/";
    for (int i = 0;i < strlen(fname);i++)
    {
        if (fname[i] == ch[(os_type + 1) % 2])
        {
            fname[i] = ch[os_type % 2];
        }
    }
}

