#ifndef STATUS_H
#define STATUS_H
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#include "config.h"
#include "instance.h"

struct ResultString {
    char* buff;
    size_t size;
};

void start_handle_status();

void handle_nodes_status();

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
char* handle_request(const char* uri, const char* method, const char* header, const char* body);
char* request_restapi_node(const char* uri);
char* request_restapi_service(const char* uri);
#endif