
#include <stdio.h>
#include "lib/log.h"
#include "lib/config.h"
#include "lib/user.h"
#include "lib/instance.h"
#include "lib/monitor.h"
#include "lib/net.h"
#include "api.h"


#if defined(_WIN32) || defined(_WIN64)
#define CONFIG_PATH "config\\config.json"
DWORD WINAPI thread_start_socket_server(LPVOID lpParam)
{
    s_log(LOG_INFO, "starting csync data server, listen on %s:%d.", base_get_data_listen_address(), base_get_data_listen_port());
    start_server_iocp(base_get_data_listen_address(), base_get_data_listen_port());
    return 0;
}

DWORD WINAPI thread_start_restapi_server(LPVOID lpParam)
{
    s_log(LOG_INFO, "starting restapi server, listen on %s:%d.", base_get_restapi_listen_address(), base_get_restapi_listen_port());
    start_restapi_server(base_get_restapi_listen_address(), base_get_restapi_listen_port());
    return 0;
}

DWORD WINAPI thread_start_monitor_server(LPVOID lpParam)
{
    s_log(LOG_INFO, "starting monitor server.");
    start_monitor_entry();
    return 0;
}
#elif defined(__linux__)
#include <pthread.h>
#define CONFIG_PATH "config/config.json"
void* thread_start_socket_server(void* lpParam)
{
    s_log(LOG_INFO, "starting csync data server, listen on %s:%d.", base_get_data_listen_address(), base_get_data_listen_port());
    start_server_linux(base_get_data_listen_address(), base_get_data_listen_port());
    return 0;
}

void* thread_start_restapi_server(void* lpParam)
{
    s_log(LOG_INFO, "starting restapi server, listen on %s:%d.", base_get_restapi_listen_address(), base_get_restapi_listen_port());
    start_restapi_server(base_get_restapi_listen_address(), base_get_restapi_listen_port());
    return 0;
}

void* thread_start_monitor_server(void* lpParam)
{
    s_log(LOG_INFO, "starting monitor server.");
    start_monitor_entry();
    return 0;
}
#else
//others
#endif



int main()
{
    set_log_params("log.txt", 0, LOG_DEBUG);
    s_log(LOG_INFO, "starting csync process.");
    load_config(CONFIG_PATH);
    load_users_config(base_get_others_path(USER));
    load_default_nodes_instances(base_get_others_path(NEIGHBOR), base_get_others_path(INSTANCE));

#if defined(_WIN32) || defined(_WIN64)
    const int NUM_THREADS = 3;
    HANDLE threads[3];

    threads[0] = CreateThread(NULL, 0, thread_start_socket_server, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, thread_start_restapi_server, NULL, 0, NULL);
    threads[2] = CreateThread(NULL, 0, thread_start_monitor_server, NULL, 0, NULL);

    Sleep(1000);
    load_instances_meta();
    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        CloseHandle(threads[i]);
    }
    s_log(LOG_INFO, "stop csync process.");
#elif defined(__linux__)
    pthread_t threads[3];
    pthread_create(&threads[0], NULL, thread_start_socket_server, NULL);
    pthread_create(&threads[1], NULL, thread_start_restapi_server, NULL);
    pthread_create(&threads[2], NULL, thread_start_monitor_server, NULL);
    const int NUM_THREADS = 3;
    sleep(1);
    load_instances_meta();
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i],NULL);
    }
    s_log(LOG_INFO, "stop csync process.");
#else
    //others
#endif

}
