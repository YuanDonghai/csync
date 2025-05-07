#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include "csync_error.h"
#include "code.h"
#include "log.h"


#if defined(_WIN32) || defined(_WIN64)
//windows
//#include <windows.h>
#include <objbase.h>

#define BASE_CONFIG "config\\config.json"
#define USERS_CONFIG "config\\users.json"
#define NEIGHBORS_CONFIG "config\\neighbors.json"
#define INSTANCES_CONFIG "config\\instances.json"

#elif defined(__linux__)
// linux
#define BASE_CONFIG "config/config.json"
#define USERS_CONFIG "config/users.json"
#define NEIGHBORS_CONFIG "config/neighbors.json"
#define INSTANCES_CONFIG "config/instances.json"
#else
//others
#endif

#define DEFAULT_LISTEN_ADDRESS "0.0.0.0"
#define RESTAPI_PORT 16345
#define DATA_PORT 26345

enum CONFIG_PATH_TYPE {
    INSTANCE,
    NEIGHBOR,
    USER
};

enum SSL_F_TYPE {
    CA_ROOT,
    SERVER_CERT,
    SERVER_KEY
};

static struct json_object* base_config;

int load_config(const char* file_path);
int load_default_base_config(const char* file_path);
char* base_get_others_path(enum CONFIG_PATH_TYPE type);
char* base_get_restapi_listen_address();
int base_get_restapi_listen_port();
char* base_get_data_listen_address();
int base_get_data_listen_port();
int get_server_cert_or_key(enum SSL_F_TYPE type, char* value);
struct json_object* get_sub_json(enum CONFIG_PATH_TYPE type);
void update_sub_json(enum CONFIG_PATH_TYPE type);
char* get_local_node_info();
const char* get_local_service();

#endif