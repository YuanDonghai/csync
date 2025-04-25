#ifndef MONITOR_H
#define MONITOR_H
#include <time.h>
#include "client.h"

#define INSTANCE_ID_LEN 64
#define INSTANCE_NAME_LEN 256
#define TASK_QUEUE_COUNTS 4096
#define INSTANCE_SOCKET_TIMEOUT_SEC 60

typedef struct {
    int type;
    DWORD action;
    char name[MAX_PATH];
    char short_name[MAX_PATH];
    char rename_name[MAX_PATH];
    int status;
}task_meta;

typedef struct {
    char id[INSTANCE_ID_LEN];
    char name[INSTANCE_NAME_LEN];
    char path[MAX_PATH];
    char peer_address[16];
    int peer_port;
    SOCKET con;
    int task_push;
    task_meta task_queues[TASK_QUEUE_COUNTS];
    time_t timestap;
    BOOL time_out_status;
    BOOL need_reconnect;
    struct instance_meta* next;
    int manual_sync_status;
}instance_meta;

enum monitor_thread_status {
    START_NEED,
    STARTED,
    STOP_NEED,
    CLEAN_NEED
};

typedef struct {
    TCHAR w_path[MAX_PATH];
    char path[MAX_PATH];
    enum monitor_thread_status status;
    struct instance_meta* instance_list;
    struct monitor_path* next;
}monitor_path;

static  monitor_path* monitor_head_p;

void start_monitor_entry();
void clean_monitor_list();

void add_instance_in_monitor(const char* id, const char* name, const char* path, const char* address, int port);

monitor_path* malloc_monitor_meta(const char* id, const char* name, const char* path, const char* address, int port);
DWORD WINAPI thread_start_monitor_directory(LPVOID lpParam);
void watch_directory(const wchar_t* directory_path, monitor_path* monitor);
void add_sync_task_in_queue_w(instance_meta* instance_p, DWORD action, const char* m_path, const wchar_t* fname,int type);
void add_sync_task_in_queue(instance_meta* instance_p, DWORD action, char* fname, char* short_name,int type);
void add_self_task_in_queue(instance_meta* instance_p, DWORD action, char* fname, char* short_name, int type);
int compare_path(const char* path1, const char* path2);

instance_meta* malloc_instance_meta(const char* id, const char* name, const char* path, const char* address, int port);
DWORD WINAPI thread_start_sync_task(LPVOID lpParam);

int check_path_type(const char* path);
long monitor_get_file_length(char* fname);

instance_meta* search_instance_p(const char* instance_id);


#endif