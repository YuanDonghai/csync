#ifndef MONITOR_H
#define MONITOR_H

#include <time.h>
#include "client.h"
#include "code.h"
#include "log.h"

#if defined(_WIN32) || defined(_WIN64)
#elif defined(__linux__)
// linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
typedef bool BOOL;
typedef int SOCKET;
typedef int DWORD;
//#define INVALID_SOCKET 0

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + NAME_MAX + 1))
#else
// others
#endif

#define INSTANCE_NAME_LEN 256
#define TASK_QUEUE_COUNTS 4096
#define INSTANCE_SOCKET_TIMEOUT_SEC 60

typedef struct
{
    int type;
    int action;
    char name[FILE_PATH_MAX_LEN];
    char short_name[FILE_PATH_MAX_LEN];
    char rename_name[FILE_PATH_MAX_LEN];
    int status;
} task_meta;

struct instance_meta
{
    char id[INSTANCE_ID_LEN];
    char peer_id[INSTANCE_ID_LEN];
    char name[INSTANCE_NAME_LEN];
    char path[FILE_PATH_MAX_LEN];
    char peer_address[IPV4_ADDRESS_LEN];
    int peer_port;
    int os_type;
    SOCKET con;
    int task_push;
    task_meta task_queues[TASK_QUEUE_COUNTS];
    time_t timestap;
    int time_out_status;
    int need_reconnect;
    struct instance_meta *next;
    int manual_sync_status;
};

enum monitor_thread_status
{
    START_NEED,
    STARTED,
    STOP_NEED,
    CLEAN_NEED
};

#if defined(_WIN32) || defined(_WIN64)
struct monitor_path
{
    TCHAR w_path[FILE_PATH_MAX_LEN];
    char path[FILE_PATH_MAX_LEN];
    enum monitor_thread_status status;
    struct instance_meta *instance_list;
    struct monitor_path *next;
};

#elif defined(__linux__)
// linux
struct inotify_wd
{
    int wd;
    char path[FILE_PATH_MAX_LEN];
    struct inotify_wd *next;
};
struct monitor_path
{
    char w_path[FILE_PATH_MAX_LEN];
    char path[FILE_PATH_MAX_LEN];
    enum monitor_thread_status status;
    struct instance_meta *instance_list;
    struct monitor_path *next;
    struct inotify_wd *wds;
};
struct wd_s
{
    int wd;
    char path[PATH_MAX];
};
#else
// others
#endif

static struct monitor_path *monitor_head_p;

void start_monitor_entry();
void clean_monitor_list();

void add_instance_in_monitor(const char *id, const char *peer_id, const char *name, const char *path, const char *address, int port);
void add_instance_in_monitor_s(struct instance_meta *instance_info);

struct monitor_path *malloc_monitor_meta(const char *id, const char *peer_id, const char *name, const char *path, const char *address, int port);
struct monitor_path *malloc_monitor_meta_s(struct instance_meta *instance_info);
#if defined(_WIN32) || defined(_WIN64)
DWORD WINAPI thread_start_monitor_directory(LPVOID lpParam);
DWORD WINAPI thread_start_sync_task(LPVOID lpParam);
void watch_directory(const wchar_t *directory_path, struct monitor_path *monitor);

void add_sync_task_in_queue_w(struct instance_meta *instance_p, int action, const char *sec_fname, const wchar_t *fname, int type);
#elif defined(__linux__)
// linux
void *thread_start_monitor_directory(void *lpParam);
void *thread_start_sync_task(void *lpParam);
void add_watch_recursive(int fd, const char *path, struct inotify_wd *inotify_wd_p);
void watch_directory(const char *directory_path, struct monitor_path *monitor);
void add_sync_task_in_queue_w(struct instance_meta *instance_p, int action, const char *sec_fname, const char *fname, int type);
void add_inotify_wd(struct inotify_wd *inotify_wd_p, int wd, char *path);
void show_inotify_wd(struct inotify_wd *inotify_wd_p);
const char *get_inotify_wd_path(struct inotify_wd *inotify_wd_p, int wd);
#else
// others
#endif

void add_sync_task_in_queue(struct instance_meta *instance_p, int action, char *fname1, char *fname2, int type);
void add_self_task_in_queue(struct instance_meta *instance_p, int action, char *fname1, char *fname2, int type);
int compare_path(const char *path1, const char *path2);

struct instance_meta *malloc_instance_meta(const char *id, const char *peer_id, const char *name, const char *path, const char *address, int port);

struct instance_meta *malloc_instance_meta_s(struct instance_meta *instance_info);
int check_path_type(const char *path);
long monitor_get_file_length(char *fname);

struct instance_meta *search_instance_p(const char *instance_id);

void modify_os_file_name(int os_type, char *fname);

#endif