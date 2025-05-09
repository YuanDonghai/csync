#include "instance.h"
int load_default_nodes_instances(const char* nodes_path, const char* instances_path)
{
    s_log(LOG_INFO, "loading nodes config %s.", nodes_path);
    sprintf_s(nodes_file_path, FILE_PATH_MAX_LEN, "%s", nodes_path);
    if (0 != load_file_to_json(&nodes_json, nodes_path))
    {
        s_log(LOG_DEBUG, "config %s is not exist, will create it.", nodes_path);
        if (0 != load_default_neighbors(nodes_path))
        {
            s_log(LOG_ERROR, "can not load default config.");
            return -1;
        }
        s_log(LOG_ERROR, "create and load default config.");
    }

    s_log(LOG_INFO, "loading instances config %s.", instances_path);
    sprintf_s(instances_file_path, FILE_PATH_MAX_LEN, "%s", instances_path);
    if (0 != load_file_to_json(&instances_json, instances_path))
    {
        if (0 != load_default_instances(instances_path))
        {
            s_log(LOG_ERROR, "can not load instances metadata.");
            return -1;
        }
        {
            return 0;
        }
    }
}

int load_default_neighbors(const char* file_path)
{
    nodes_json = json_object_new_array();
    // save
    dump_json_to_file(nodes_json, file_path);
    return 0;
}

int load_default_instances(const char* file_path)
{
    instances_json = json_object_new_object();
    json_object_object_add(instances_json, "workspaces", json_object_new_array());
    json_object_object_add(instances_json, "instances", json_object_new_array());
    // save
    dump_json_to_file(instances_json, file_path);
}

struct json_object* get_json_obj(int type)
{
    switch (type)
    {
    case 0:
        return nodes_json;
        break;
    case 1:
        return workspace_list;
        break;
    case 2:
        return instances_list;
        break;
    default:
        return NULL;
        break;
    }
}
void save_json(int type)
{
    switch (type)
    {
    case 0:
        dump_json_to_file(nodes_json, nodes_file_path);
        break;
    case 1:
        dump_json_to_file(instances_json, instances_file_path);
        break;

    default:

        break;
    }
}

char* _node_get_nodes()
{
    return json_object_get_string(nodes_json);
}

char* _node_add_nodes(const char* body_json)
{
    char temp_str[4096];
    char name[4096];
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    struct json_object* add_obj_address;
    if (!json_object_object_get_ex(add_new_obj, "address", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no address\"}";
    }
    sprintf_s(temp_str, 4096, "https://%s:16345", json_object_get_string(add_obj_address));
    sprintf_s(name, 4096, "%s", json_object_get_string(add_obj_address));
    if (!json_object_object_get_ex(add_new_obj, "api_url", &add_obj_address))
    {
        json_object_object_add(add_new_obj, "api_url", json_object_new_string(temp_str));
    }

    int array_length = json_object_array_length(nodes_json);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(nodes_json, i);
        struct json_object* pass_obj;
        if (json_object_object_get_ex(element, "address", &pass_obj))
        {
            if (0 == strcmp(name, json_object_get_string(pass_obj)))
            {
                return "{\"result\":\"error\",\"error\":\"this address exist\"}";
            }
        }
    }

    json_object_object_add(add_new_obj, "id", json_object_new_string(""));
    json_object_object_add(add_new_obj, "name", json_object_new_string(name));
    json_object_object_add(add_new_obj, "sync_port", json_object_new_int(26345));
    json_object_object_add(add_new_obj, "status", json_object_new_string("adding"));
    if (0 == json_object_array_add(nodes_json, add_new_obj))
    {
        dump_json_to_file(nodes_json, nodes_file_path);
    }

    return "{}";
}

int get_node_address_port(char* node_id, char* address, int* port, int* os_type)
{
    int array_length = json_object_array_length(nodes_json);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* value_obj = json_object_array_get_idx(nodes_json, i);
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
                if (json_object_object_get_ex(value_obj, "os_type", &node_value_obj))
                {
                    *os_type = json_object_get_int(node_value_obj);
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
    if (json_object_object_get_ex(nodes_json, node_id, &value_obj))
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

void load_instances_meta()
{

    // workspace and instance list
    workspace_list = json_object_new_array();
    instances_list = json_object_new_array();
    if (json_object_object_get_ex(instances_json, "workspaces", &workspace_list))
    {
        s_log(LOG_DEBUG, "load workspace list.");
    }
    else
    {
        s_log(LOG_DEBUG, "load workspace list, workspace is empty.");
        return;
    }
    if (json_object_object_get_ex(instances_json, "instances", &instances_list))
    {
        s_log(LOG_DEBUG, "load instance list.");
    }
    else
    {
        s_log(LOG_DEBUG, "load instance list, instance is empty.");
        return;
    }

    // push instances
    /*
    char in_id[256];
    char peer_id[256];
    char in_name[256];
    char in_node_id[256];
    char in_peer_addr[256];
    char wss_id[256];
    char path[FILE_PATH_MAX_LEN];
    int port;
    int type;
*/
    char wss_id[256];
    char in_node_id[256];
    struct instance_meta* push_ins = (struct instance_meta*)malloc(sizeof(struct instance_meta));
    int array_length = json_object_array_length(instances_list);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* in_val = json_object_array_get_idx(instances_list, i);
        struct json_object* in_value_obj;

        if (json_object_object_get_ex(in_val, "id", &in_value_obj))
        {
            sprintf_s(push_ins->id, INSTANCE_ID_LEN, "%s", json_object_get_string(in_value_obj));
        }
        else
        {
            continue;
        }

        if (json_object_object_get_ex(in_val, "type", &in_value_obj))
        {
            if (0 == json_object_get_int(in_value_obj))
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
            sprintf_s(push_ins->peer_id, INSTANCE_ID_LEN, "%s", json_object_get_string(in_value_obj));
        }
        else
        {
            continue;
        }
        if (json_object_object_get_ex(in_val, "name", &in_value_obj))
        {
            sprintf_s(push_ins->name, INSTANCE_NAME_LEN, "%s", json_object_get_string(in_value_obj));
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
        /*
        if (0 != get_node_address_port(in_node_id, in_peer_addr, &port))
        {
            continue;
        }
        */
        if (0 != get_node_address_port(in_node_id, push_ins->peer_address, &(push_ins->peer_port), &(push_ins->os_type)))
        {
            continue;
        }

        sprintf_s(push_ins->path, FILE_PATH_MAX_LEN, "%s", get_workspace_path(wss_id));
        if (0 < strlen(push_ins->path))
        {
            // add_instance_in_monitor(in_id, peer_id,in_name, path, in_peer_addr, port);
            add_instance_in_monitor_s(push_ins);
            _sleep_or_Sleep(1);
        }
        else
        {
            continue;
        }
    }
    free(push_ins);
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
                    sprintf_s(path, 256, "%s", get_workspace_path(json_object_get_string(id_obj)));
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
}

char* _instance_get_workspace()
{
    return json_object_get_string(workspace_list);
}
char* _instance_add_workspace(const char* body_json)
{
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    struct json_object* add_obj_address;
    if (!json_object_object_get_ex(add_new_obj, "name", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no name\"}";
    }
    if (!json_object_object_get_ex(add_new_obj, "path", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no path\"}";
    }

    char ch_add_path[FILE_PATH_MAX_LEN];
    char exist_path[FILE_PATH_MAX_LEN];
    char exist_name[FILE_PATH_MAX_LEN];
    sprintf_s(ch_add_path, FILE_PATH_MAX_LEN, "%s", json_object_get_string(add_obj_address));
    format_path(ch_add_path);
    int array_length = json_object_array_length(workspace_list);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(workspace_list, i);
        struct json_object* pass_obj;
        if (json_object_object_get_ex(element, "path", &pass_obj))
        {
            sprintf_s(exist_path, FILE_PATH_MAX_LEN, "%s", json_object_get_string(pass_obj));
            format_path(exist_path);
            if (0 == strcmp(ch_add_path, exist_path))
            {
                return "{\"result\":\"error\",\"error\":\"this address exist\"}";
            }
        }
    }
    json_object_object_add(add_new_obj, "id", json_object_new_string(gen_uuid_str()));

    if (0 == json_object_array_add(workspace_list, add_new_obj))
    {
        dump_json_to_file(instances_json, instances_file_path);
    }
    return "{}";
}

char* _instance_get_instance()
{
    return json_object_get_string(instances_list);
}
char* _instance_add_instance(const char* body_json)
{
    /*
    {
    name
    wss
    type
    nodes :[]
    }
    */
    char wss_id[128];
    int type = 0;
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    struct json_object* add_obj_address;
    if (!json_object_object_get_ex(add_new_obj, "wss", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no worspace\"}";
    }
    sprintf_s(wss_id, 128, "%s", json_object_get_string(add_obj_address));

    if (!json_object_object_get_ex(add_new_obj, "type", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no type\"}";
    }
    type = json_object_get_int(add_obj_address);

    if (!json_object_object_get_ex(add_new_obj, "nodes", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no nodes\"}";
    }
    else
    {
        if (0 == json_object_array_length(add_obj_address))
        {
            return "{\"result\":\"error\",\"error\":\"no nodes\"}";
        }
    }

    int array_length = json_object_array_length(add_obj_address);
    char ch_node[128];

    for (int i = 0; i < array_length; i++)
    {
        /*
          "id": "EE386E60-9B14-4390-8555-F31F05901144",
            "wss_id": "EE386E60-9B14-4390-8555-F31F05901A41",
            "name": "csync0_instance",
            "peer_node": "4F0E6814-28D4-416F-B741-94402C359D0B",
            "peer_iid": "EE386E60-9B14-4390-8555-F31F05901B34",
            "type": 2
        */
        struct json_object* element = json_object_array_get_idx(add_obj_address, i);
        sprintf_s(ch_node, 128, "%s", json_object_get_string(element));
        struct json_object* add_new_ins = json_object_new_object();
        json_object_object_add(add_new_ins, "id", json_object_new_string(gen_uuid_str()));
        json_object_object_add(add_new_ins, "wss_id", json_object_new_string(wss_id));
        json_object_object_add(add_new_ins, "peer_node", json_object_new_string(ch_node));
        json_object_object_add(add_new_ins, "type", json_object_new_int(type));
        json_object_object_add(add_new_ins, "status", json_object_new_string("adding"));
        json_object_array_add(instances_list, add_new_ins);


    }

    dump_json_to_file(instances_json, instances_file_path);

    return "{}";
}

char* _instance_connect_instance(const char* body_json)
{
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    json_object_object_add(add_new_obj, "peer_iid", json_object_new_string(gen_uuid_str()));

    // 本地创建instance，添加event。

    return json_object_get_string(add_new_obj);
}