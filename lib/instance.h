#ifndef INSTANCE_H
#define INSTANCE_H
#include <windows.h>
#include <stdio.h>
#include <json-c/json.h>
//#include "monitor.h"
#include "log.h"
#include "config.h"
#include "code.h"

static struct json_object* neighbors_json;
static struct json_object* instances_json;
static struct json_object* workspace_list;
static struct json_object* instances_list;
void set_instances_json_link(struct json_object* json_neighbor,struct json_object* json_instance);
char* get_workspace_path(const char* ws_id);
int get_node_address_port(char * node_id,char *address,int *port);
void get_instance_path(const char* id,char *path);

char* _instance_get_workspace();
char* _instance_add_workspace(const char* body_json);
char* _instance_get_instance();
char* _instance_add_instance(const char* body_json);

#endif
