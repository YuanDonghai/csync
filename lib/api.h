#ifndef API_SERVER_H
#define API_SERVER_H
#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "config.h"
#include "user.h"
#include "instance.h"

#define AUTH_ENABLE 1
#define VERSION "v1"
#define MAX_LOGIN_TOKENS 64

enum api_controller_path {
    API_ROOT,
    API_LOGIN,
    API_NODE,
    API_SERVICE,
    API_NODES, 
    API_WSS,
    API_INSTANCES,
    INDEX_END
};
#define _API_COUNTS_I INDEX_END+1
static char* api_controller[] = {
    "/api",
    "/api/login",
    "/api/node",
    "/api/service",
    "/api/nodes",
    "/api/wss",
    "/api/instances",
    "/"
};

enum api_method {
    M_GET,
    M_POST,
    M_PUT,
    M_DELETE,
    METHOD_END
};
#define _METHOD_COUNTS_I METHOD_END+1
static char* api_methods[] = {
    "GET",
    "POST",
    "PUT",
    "DELETE"
};


static char auth_tls_ca_cert[4096];
static char auth_tls_server_cert[4096];
static char auth_tls_server_key[4096];
static char rest_api_pass[64];

static char tokens[MAX_LOGIN_TOKENS][129];

void start_restapi_server(const char* listen_address, int port);
static void ev_handler(struct mg_connection* c, int ev, void* ev_data);

int authenticate(struct mg_http_message* hm);
int authenticate_token(const char token);

void ev_handler_path(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);
int search_route_index(struct mg_http_message* hm);
int search_method_index(struct mg_http_message* hm);

//others
void default_path(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);

// api root
void api_root(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);
// api login
void handle_login(struct mg_connection* c, struct mg_http_message* hm, void* ev_dat);
// api service
void api_service(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);
// api nodes
void api_node(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);
// api neighbor
void api_nodes(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);
// api workspace
void api_wss(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);
// api instance
void api_instances(struct mg_connection* c, struct mg_http_message* hm, void* ev_data);

#endif