#ifndef SYNC_NODE_H
#define SYNC_NODE_H
#include <json-c/json.h>

#include "code.h"

static struct json_object* neighbors_json;
void set_nodes_json_link(struct json_object* json_p);

char* _node_get_nodes();
char* _node_add_nodes(const char* body_json);

#endif