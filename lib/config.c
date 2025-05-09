#include "config.h"

int load_config(const char* file_path)
{
    s_log(LOG_INFO, "loading config %s.", file_path);
    if (0 != load_file_to_json(&base_config, file_path))
    {
        s_log(LOG_INFO, "config %s is not exist, will create it.", file_path);
        if (0 != load_default_base_config(file_path))
        {
            s_log(LOG_ERROR, "can not load default config.");
            return -1;
        }
        s_log(LOG_ERROR, "create and load default config.");
    }
    return 0;
}

int load_default_base_config(const char* file_path)
{
    base_config = json_object_new_object();
    struct json_object* add_new_obj;
    // node
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "name", json_object_new_string(get_hostname()));
    json_object_object_add(add_new_obj, "id", json_object_new_string(gen_uuid_str()));
    //json_object_object_add(add_new_obj, "name", json_object_new_string("localhost.localdomain"));   
    //json_object_object_add(add_new_obj, "id", json_object_new_string("0f1a0516-1456-41c9-b386-fbf5bd78c223"));
    printf(" config:\n%s\n", json_object_get_string(add_new_obj));
    if (0 != json_object_object_add(base_config, "node", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // certs 
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "ca", json_object_new_string(""));
    json_object_object_add(add_new_obj, "server_cert", json_object_new_string(""));
    json_object_object_add(add_new_obj, "server_key", json_object_new_string(""));
    if (0 != json_object_object_add(base_config, "certs", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // rest api
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "port", json_object_new_int(RESTAPI_PORT));
    json_object_object_add(add_new_obj, "listen_address", json_object_new_string(DEFAULT_LISTEN_ADDRESS));
    if (0 != json_object_object_add(base_config, "restapi", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    //sync server
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "port", json_object_new_int(DATA_PORT));
    json_object_object_add(add_new_obj, "listen_address", json_object_new_string(DEFAULT_LISTEN_ADDRESS));
    if (0 != json_object_object_add(base_config, "sync", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // user
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "metadata", json_object_new_string(USERS_CONFIG));
    if (0 != json_object_object_add(base_config, "users", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // neighbor
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "metadata", json_object_new_string(NEIGHBORS_CONFIG));
    if (0 != json_object_object_add(base_config, "neighbors", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // instances
    add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "metadata", json_object_new_string(INSTANCES_CONFIG));
    if (0 != json_object_object_add(base_config, "instances", add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // save
    dump_json_to_file(base_config, file_path);
}

char* base_get_others_path(enum CONFIG_PATH_TYPE type)
{
    char ch_type[256];
    char ch_type_default[256];
    struct json_object* node_obj;
    switch (type)
    {
    case INSTANCE:
        sprintf_s(ch_type, 256, "instances");
        sprintf_s(ch_type_default, 256, "%s", INSTANCES_CONFIG);
        break;
    case NEIGHBOR:
        sprintf_s(ch_type, 256, "neighbors");
        sprintf_s(ch_type_default, 256, "%s", NEIGHBORS_CONFIG);
        break;
    case USER:
        sprintf_s(ch_type, 256, "users");
        sprintf_s(ch_type_default, 256, "%s", USERS_CONFIG);
        break;
    default:
        return "";
        break;
    }
    if (json_object_object_get_ex(base_config, ch_type, &node_obj))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(node_obj, "metadata", &jvalue))
        {
            return json_object_get_string(jvalue);
        }
        else
        {
            return ch_type_default;
        }
    }
    else
    {
        return ch_type_default;
    }
}

char* base_get_restapi_listen_address()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "restapi", &node_obj))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(node_obj, "listen_address", &jvalue))
        {
            return json_object_get_string(jvalue);
        }
        else
        {
            return DEFAULT_LISTEN_ADDRESS;
        }
    }
    else
    {
        return DEFAULT_LISTEN_ADDRESS;
    }
}

int base_get_restapi_listen_port()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "restapi", &node_obj))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(node_obj, "port", &jvalue))
        {
            return json_object_get_int(jvalue);
        }
        else
        {
            return RESTAPI_PORT;
        }
    }
    else
    {
        return RESTAPI_PORT;
    }
}

char* base_get_data_listen_address()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "sync", &node_obj))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(node_obj, "listen_address", &jvalue))
        {
            return json_object_get_string(jvalue);
        }
        else
        {
            return DEFAULT_LISTEN_ADDRESS;
        }
    }
    else
    {
        return DEFAULT_LISTEN_ADDRESS;
    }
}

int base_get_data_listen_port()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "sync", &node_obj))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(node_obj, "port", &jvalue))
        {
            return json_object_get_int(jvalue);
        }
        else
        {
            return DATA_PORT;
        }
    }
    else
    {
        return DATA_PORT;
    }
}

int get_server_cert_or_key(enum SSL_F_TYPE type, char* values)
{
    char ch_fname[1024];
    memset(ch_fname, 0, 1024);
    char ch_type[20];
    memset(ch_type, 0, 20);
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "certs", &node_obj))
    {
        struct json_object* value_obj;
        switch (type)
        {
        case CA_ROOT:
            sprintf_s(ch_type, 20, "ca");
            break;
        case SERVER_CERT:
            sprintf_s(ch_type, 20, "server_cert");
            break;
        case SERVER_KEY:
            sprintf_s(ch_type, 20, "server_key");
            break;
        default:
            return 1;
            break;
        }
        if (json_object_object_get_ex(node_obj, ch_type, &value_obj))
        {
            sprintf_s(ch_fname, 1024, "%s", json_object_get_string(value_obj));
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }

    FILE* file = fopen(ch_fname, "r");
    if (!file)
    {
        s_log(LOG_DEBUG, "config open file error. %s.", ch_fname);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(values, 1, len, file);
    fclose(file);
    return 0;
}

char* get_local_node_info()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "node", &node_obj))
    {
#if defined(_WIN32) || defined(_WIN64)
        json_object_object_add(node_obj, "os_type", json_object_new_int(0));
#elif defined(__linux__)
        json_object_object_add(node_obj, "os_type", json_object_new_int(1));
#else
        //others
#endif
        return json_object_get_string(node_obj);
    }
    else
    {
        return "{}";
    }
}

void get_local_service(char* res)
{
    //const char res_string[4096];
    char rest_addr[32];
    int rest_port = 0;
    char sync_addr[32];
    int sync_port = 0;
    struct json_object* node_obj;
    struct json_object* value_obj;
    if (json_object_object_get_ex(base_config, "restapi", &node_obj))
    {
        if (json_object_object_get_ex(node_obj, "listen_address", &value_obj))
        {
            sprintf_s(rest_addr, 32, "%s", json_object_get_string(value_obj));
        }
        else
        {
            sprintf_s(rest_addr, 32, "0.0.0.0");
        }
        if (json_object_object_get_ex(node_obj, "port", &value_obj))
        {
            rest_port = json_object_get_int(value_obj);
        }
        else
        {
            rest_port = 16345;
        }
    }
    if (json_object_object_get_ex(base_config, "sync", &node_obj))
    {
        if (json_object_object_get_ex(node_obj, "listen_address", &value_obj))
        {
            sprintf_s(sync_addr, 32, "%s", json_object_get_string(value_obj));
        }
        else
        {
            sprintf_s(sync_addr, 32, "0.0.0.0");
        }
        if (json_object_object_get_ex(node_obj, "port", &value_obj))
        {
            sync_port = json_object_get_int(value_obj);
        }
        else
        {
            sync_port = 16345;
        }
    }

    sprintf_s(res, 4096, "{\"restapi\":{\"port\":%d,\"listen_address\" : \"%s\"},\"sync\":{\"port\":%d,\"listen_address\" : \"%s\"}}", rest_port, rest_addr, sync_port, sync_addr);
    // s_log(LOG_DEBUG, "get_local_service %s.", res);
     // return res_string;
}
