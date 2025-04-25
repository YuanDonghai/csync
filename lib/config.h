#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <objbase.h>
#include <json-c/json.h>

#include "csync_error.h"
#include "code.h"
#include "log.h"

#define DEFAULT_LISTEN_ADDRESS "0.0.0.0"
#define BASE_CONFIG "config\\config.json"
#define USERS_CONFIG "config\\users.json"
#define NEIGHBORS_CONFIG "config\\neighbors.json"
#define INSTANCES_CONFIG "config\\instances.json"
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
static struct json_object* instances_config;
static struct json_object* users_config;
static struct json_object* neighbors_config;
/*
base config
*/
int load_config(const char* file_path);
int load_default_base_config(const char* file_path);
char* base_get_instance_path();
char* base_get_others_path(enum CONFIG_PATH_TYPE type);
char* base_get_restapi_listen_address();
int base_get_restapi_listen_port();
char* base_get_data_listen_address();
int base_get_data_listen_port();

struct json_object* get_sub_json(enum CONFIG_PATH_TYPE type);
void update_sub_json(enum CONFIG_PATH_TYPE type);
int get_server_cert_or_key(enum SSL_F_TYPE type, char* value);
int get_api_server_user(char* user, char* pass);
char* get_local_node_info();
char* get_local_service();

/*
instance
*/
int load_default_instances(const char* file_path);
/*
void add_instance(const char* ws_id, const char* id, const char* name, const char* path);
char* get_instance_path(const char* id);
int get_instance_counts();
*/
/*
users
*/
int load_default_users(const char* file_path);

/*
neighbors
*/
int load_default_neighbors(const char* file_path);


int load_file_to_json(struct json_object** json_data, const char* file_path);
int dump_json_to_file(struct json_object* json_data, const char* file_path);

/*extends*/
char* get_hostname();

#endif