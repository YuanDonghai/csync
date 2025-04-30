#ifndef SYNC_USER_H
#define SYNC_USER_H
#include <json-c/json.h>
#include "code.h"
#include "log.h"

static struct json_object* user_json;

int load_users_config(const char* file_path);
int load_default_users(const char* file_path);
int get_api_server_user(char* user, char* pass);
#endif