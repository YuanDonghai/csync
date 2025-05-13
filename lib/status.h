#ifndef STATUS_H
#define STATUS_H
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#include "config.h"
#include "instance.h"
#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#elif defined(__linux__)
// linux
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET  (int)(~0)
#define SOCKET_ERROR            (-1)

#else
//others
#endif

struct ResultString {
    char* buff;
    size_t size;
};

void start_handle_status();

void handle_nodes_status();
void handle_instances_status();

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
char* handle_request(const char* uri, const char* method, const char* header, const char* body);
char* request_restapi_node(const char* uri);
char* request_restapi_service(const char* uri);
char* request_restapi_negotiate_node(const char* uri);

char* request_restapi_negotiate_instance(const char* uri, const char* body_json);


#endif