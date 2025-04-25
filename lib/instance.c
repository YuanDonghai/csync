#include "instance.h"
void set_instances_json_link(struct json_object* json_neighbor, struct json_object* json_instance)
{
    neighbors_json = json_neighbor;
    instances_json = json_instance;

    // workspace and instance list
    workspace_list = json_object_new_array();
    instances_list = json_object_new_array();
    if (json_object_object_get_ex(instances_json, "workspaces", &workspace_list))
    {
        s_log(LOG_DEBUG, "load workspace list.");
    }
    else
    {
        return;
    }
    if (json_object_object_get_ex(instances_json, "instances", &instances_list))
    {
        s_log(LOG_DEBUG, "load instance list.");
    }
    else
    {
        return;
    }

    // push instances
    char in_id[256];
    char in_name[256];
    char in_node_id[256];
    char in_peer_addr[256];
    char wss_id[256];
    char path[MAX_PATH];
    int port;
    int type;

    int array_length = json_object_array_length(instances_list);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* in_val = json_object_array_get_idx(instances_list, i);
        struct json_object* in_value_obj;
        if (json_object_object_get_ex(in_val, "type", &in_value_obj))
        {
            type = json_object_get_int(in_value_obj);
            if (type == 0)
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        if (json_object_object_get_ex(in_val, "peer_iid", &in_value_obj))
        {
            sprintf_s(in_id, 256, "%s", json_object_get_string(in_value_obj));
        }
        else
        {
            continue;
        }
        if (json_object_object_get_ex(in_val, "name", &in_value_obj))
        {
            sprintf_s(in_name, 256, "%s", json_object_get_string(in_value_obj));
        }
        else
        {
            continue;
        }
        if (json_object_object_get_ex(in_val, "wss_id", &in_value_obj))
        {
            sprintf_s(wss_id, 256, "%s", json_object_get_string(in_value_obj));
        }
        else
        {
            continue;
        }
        if (json_object_object_get_ex(in_val, "peer_node", &in_value_obj))
        {
            sprintf_s(in_node_id, 256, "%s", json_object_get_string(in_value_obj));
        }
        else
        {
            continue;
        }
        if (0 != get_node_address_port(in_node_id, in_peer_addr, &port))
        {
            continue;
        }

        sprintf_s(path, MAX_PATH, "%s", get_workspace_path(wss_id));
        if (0 < strlen(path))
        {
            s_log(LOG_INFO, "loading instance: %s, name=%s, path=%s, node=%s:%d.", in_id, in_name, path, in_peer_addr, port);
            add_instance_in_monitor(in_id, in_name, path, in_peer_addr, port);
            Sleep(1);
        }
        else
        {
            continue;
        }
    }
}

char* get_workspace_path(const char* ws_id)
{
    int array_length = json_object_array_length(workspace_list);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(workspace_list, i);
        struct json_object* id_obj;
        if (json_object_object_get_ex(element, "id", &id_obj))
        {
            if (0 == strcmp(ws_id, json_object_get_string(id_obj)))
            {
                if (json_object_object_get_ex(element, "path", &id_obj))
                {
                    return json_object_get_string(id_obj);
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }
    }
    return "";
}

int get_node_address_port(char* node_id, char* address, int* port)
{
    int array_length = json_object_array_length(neighbors_json);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* value_obj = json_object_array_get_idx(neighbors_json, i);
        struct json_object* node_value_obj;
        if (json_object_object_get_ex(value_obj, "id", &node_value_obj))
        {
            if (0 != strcmp(node_id, json_object_get_string(node_value_obj)))
            {
                continue;
            }
            else
            {
                if (json_object_object_get_ex(value_obj, "address", &node_value_obj))
                {
                    sprintf_s(address, 256, "%s", json_object_get_string(node_value_obj));
                }
                else
                {
                    return -1;
                }
                if (json_object_object_get_ex(value_obj, "sync_port", &node_value_obj))
                {
                    *port = json_object_get_int(node_value_obj);
                }
                else
                {
                    return -1;
                }
                return 0;
            }
        }


    }

    struct json_object* value_obj;
    if (json_object_object_get_ex(neighbors_json, node_id, &value_obj))
    {
        struct json_object* node_value_obj;
        if (json_object_object_get_ex(value_obj, "address", &node_value_obj))
        {
            sprintf_s(address, 256, "%s", json_object_get_string(node_value_obj));
        }
        else
        {
            return -1;
        }
        if (json_object_object_get_ex(value_obj, "sync_port", &node_value_obj))
        {
            *port = json_object_get_int(node_value_obj);
        }
        else
        {
            return -1;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

void get_instance_path(const char* id, char* path)
{
    int array_length = json_object_array_length(instances_list);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(instances_list, i);
        struct json_object* id_obj;
        if (json_object_object_get_ex(element, "id", &id_obj))
        {
            if (0 == strcmp(id, json_object_get_string(id_obj)))
            {
                if (json_object_object_get_ex(element, "wss_id", &id_obj))
                {
                    sprintf_s(path,256,"%s",get_workspace_path(json_object_get_string(id_obj)));
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }
    }
    return "";
}

char* _instance_get_workspace()
{
    return json_object_get_string(workspace_list);
}
char* _instance_add_workspace(const char* body_json)
{
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    struct json_object* add_obj_address;
    if (!json_object_object_get_ex(add_new_obj, "path", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no path\"}";
    }
    char ch_add_path[MAX_PATH];
    char exist_path[MAX_PATH];
    sprintf_s(ch_add_path, MAX_PATH, "%s", json_object_get_string(add_obj_address));
    format_path(ch_add_path);
    int array_length = json_object_array_length(workspace_list);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(workspace_list, i);
        struct json_object* pass_obj;
        if (json_object_object_get_ex(element, "path", &pass_obj))
        {
            sprintf_s(exist_path, MAX_PATH, "%s", json_object_get_string(pass_obj));
            format_path(exist_path);
            if (0 == strcmp(ch_add_path, exist_path))
            {
                return "{\"result\":\"error\",\"error\":\"this address exist\"}";
            }
        }
    }
    json_object_object_add(add_new_obj, "id", json_object_new_string(gen_node_id()));

    if (0 == json_object_array_add(workspace_list, add_new_obj))
    {
        update_sub_json(0);
    }
    return "{}";
}

char* _instance_get_instance()
{
    return json_object_get_string(instances_list);
}
char* _instance_add_instance(const char* body_json)
{
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    json_object_object_add(add_new_obj, "id", json_object_new_string(gen_node_id()));
    json_object_object_add(add_new_obj, "peer_iid", json_object_new_string(gen_node_id()));

    if (0 == json_object_array_add(instances_list, add_new_obj))
    {
        update_sub_json(0);
    }
    return "{}";
}

