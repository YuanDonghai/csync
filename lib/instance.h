#ifndef INSTANCE_H
#define INSTANCE_H

#include <stdio.h>
#include <json-c/json.h>

#if defined(_WIN32) || defined(_WIN64)
//#include <windows.h>
#elif defined(__linux__)
// linux
#else
//others
#endif

#include "monitor.h"
#include "log.h"
#include "config.h"
#include "code.h"

static char nodes_file_path[FILE_PATH_MAX_LEN];
static char instances_file_path[FILE_PATH_MAX_LEN];
static struct json_object* nodes_json;
static struct json_object* instances_json;
static struct json_object* workspace_list;
static struct json_object* instances_list;

int load_default_nodes_instances(const char* nodes_path, const char* instances_path);
int load_default_neighbors(const char* file_path);
int load_default_instances(const char* file_path);

/*
nodes
*/
char* _node_get_nodes();
char* _node_add_nodes(const char* body_json);
int get_node_address_port(char* node_id, char* address, int* port, int* os_type);
/*
instances
*/
char* _instance_get_workspace();
char* _instance_add_workspace(const char* body_json);
char* _instance_get_instance();
char* _instance_add_instance(const char* body_json);
void load_instances_meta();
char* get_workspace_path(const char* ws_id);
void get_instance_path(const char* id, char* path);

#endif
