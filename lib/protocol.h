#ifndef SYNC_PROTOCOL_H
#define SYNC_PROTOCOL_H
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <librsync.h>

#if defined(_WIN32) || defined(_WIN64)
//#include <ws2tcpip.h>
#elif defined(__linux__)
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
// linux
#else
//others
#endif

#include "diff.h"
#include "md5.h"
#include "code.h"
#include "log.h"
#include "instance.h"
#include "csync_error.h"
#include "protocol_sm.h"
#include "monitor.h"
#include "file_extend.h"

#define FILE_NAME_MAX_LENGTH 4096
#define CHECK_SUM_LEN 32+1
#define RESP_DATA_MAX_LENGTH 4096
#define BIG_CACHE_SIZE  1024 * 1024 *64
#define FILE_TIME_DIFF 1
//#define INSTANCE_ID_LEN 32

#if defined(_WIN32) || defined(_WIN64)
#define PATH_ADD_STRING "\\"
#elif defined(__linux__)
#define PATH_ADD_STRING "/"
#else
#endif

enum client_type
{
    NULL_OPT = 0,
    CLI_REQ_SIG = 1
};

typedef struct
{
    long index;
    int socket;
    char client_address[IPV4_ADDRESS_LEN];
    int client_port;
    enum sync_status status;
    int using_status;
    char* data;
    long data_len;
    char instance_id[INSTANCE_ID_LEN];
    char instance_path[FILE_NAME_MAX_LENGTH];
    char file_name[FILE_NAME_MAX_LENGTH];
    time_t file_time;
    char patch_name[FILE_NAME_MAX_LENGTH];
    char cache_path[FILE_NAME_MAX_LENGTH];
    char sig_name[FILE_NAME_MAX_LENGTH];
    char delta_name[FILE_NAME_MAX_LENGTH];
    char* next_data;
    long next_data_len;
    __int64 will_recv_data_len;
    char will_recv_checksum[CHECK_SUM_LEN];
    __int64 data_recv_data_len;
    size_t recv_counts;
    size_t send_counts;
    char* big_cache;
    size_t big_cache_counts;
    int big_cache_malloc;
    struct instance_meta* instance_p;

} sync_protocol;

int push_stream_to_data(char* data, unsigned long len, sync_protocol* protocol);
void post_update_status(sync_protocol* protocol);
void reset_status(sync_protocol* protocol);
void set_protocol_status_error(sync_protocol* protocol);
void set_protocol_status_ok(enum sync_status status, sync_protocol* protocol);

int trans_status_on_path(char* data, unsigned long len, sync_protocol* protocol);
int trans_status_on_ready(char* data, unsigned long len, sync_protocol* protocol);
int trans_status_on_ack_sig(char* data, unsigned long len, sync_protocol* protocol);
int trans_status_on_req_send_del(char* data, unsigned long len, sync_protocol* protocol);
int trans_status_on_ack_send_del(char* data, unsigned long len, sync_protocol* protocol);
int trans_status_on_ack_del(char* data, unsigned long len, sync_protocol* protocol);
int trans_status_on_recv_new(char* data, unsigned long len, sync_protocol* protocol);

int update_instance(const char* instance_id, sync_protocol* protocol);
int check_locals_time(sync_protocol* protocol);

#endif