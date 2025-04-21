
#include <stdio.h>
// #include <windows.h>
#include "lib/net.h"
#include "lib/config.h"
#include "lib/api.h"
#include "lib/log.h"
//#include "lib/instance.h"
#include "lib/monitor.h"

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

int main()
{
    set_log_params("log.txt", 0, LOG_DEBUG);
    s_log(LOG_INFO, "starting csync process.");

    load_config("config.json");

    int instance_counts = get_instance_counts();
    const int NUM_THREADS = 2;
    HANDLE threads[3];

    threads[0] = CreateThread(NULL, 0, thread_start_socket_server, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, thread_start_restapi_server, NULL, 0, NULL);
    threads[2] = CreateThread(NULL, 0, thread_start_monitor_server, NULL, 0, NULL);

    add_instance_in_monitor("instanceid1", "test1", "D:\\WORKDIR\\csync0", "127.0.0.1", 26345);
    add_instance_in_monitor("instanceid2", "test2", "D:\\WORKDIR\\csync0", "127.0.0.1", 26345);
    // start_monitor_entry();
    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    for (int i = 0; i < NUM_THREADS; i++)
    {
        CloseHandle(threads[i]);
    }
    s_log(LOG_INFO, "stop csync process.");
}
