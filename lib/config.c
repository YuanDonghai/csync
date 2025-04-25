#include "config.h"
//struct All_config global_config;


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

    s_log(LOG_INFO, "loading users config %s.", base_get_others_path(USER));
    if (0 != load_file_to_json(&users_config, base_get_others_path(USER)))
    {
        if (0 != load_default_users(base_get_others_path(USER)))
        {
            s_log(LOG_ERROR, "can not load users metadata.");
            return -1;
        }
    }

    s_log(LOG_INFO, "loading neighbors config %s.", base_get_others_path(NEIGHBOR));
    if (0 != load_file_to_json(&neighbors_config, base_get_others_path(NEIGHBOR)))
    {
        if (0 != load_default_neighbors(base_get_others_path(NEIGHBOR)))
        {
            s_log(LOG_ERROR, "can not load neighbors metadata.");
            return -1;
        }
    }

    s_log(LOG_INFO, "loading instances config %s.", base_get_others_path(INSTANCE));
    if (0 != load_file_to_json(&instances_config, base_get_others_path(INSTANCE)))
    {
        if (0 != load_default_instances(base_get_others_path(INSTANCE)))
        {
            s_log(LOG_ERROR, "can not load instances metadata.");
            return -1;
        }
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
    json_object_object_add(add_new_obj, "id", json_object_new_string(gen_node_id()));
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

char* base_get_instance_path()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "instance", &node_obj))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(node_obj, "metadata", &jvalue))
        {
            return json_object_get_string(jvalue);
        }
        else
        {
            return INSTANCES_CONFIG;
        }
    }
    else
    {
        return INSTANCES_CONFIG;
    }
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

struct json_object* get_sub_json(enum CONFIG_PATH_TYPE type)
{
    switch (type)
    {
    case INSTANCE:
        return instances_config;
        break;
    case NEIGHBOR:
        return neighbors_config;
        break;
    case USER:
        return users_config;
        break;
    default:
        return NULL;
        break;
    }
    return NULL;
}

void update_sub_json(enum CONFIG_PATH_TYPE type)
{
    dump_json_to_file(get_sub_json(type), base_get_others_path(type));
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
        s_log(LOG_DEBUG, "[server] config open file error. %s.", ch_fname);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(values, 1, len, file);
    fclose(file);
    return 0;
}

int get_api_server_user(char* user, char* pass)
{
    if (!json_object_is_type(users_config, json_type_array))
    {
        s_log(LOG_ERROR, "the user config is not an array.");
        return 1;
    }
    int array_length = json_object_array_length(users_config);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(users_config, i);
        struct json_object* pass_obj;
        if (json_object_object_get_ex(element, "password", &pass_obj))
        {
            sprintf_s(pass, 64, "%s", json_object_get_string(pass_obj));
            return 0;
        }      
    }
    return 1;  
}

char* get_local_node_info()
{
    struct json_object* node_obj;
    if (json_object_object_get_ex(base_config, "node", &node_obj))
    {
        return json_object_get_string(node_obj);
    }
    else
    {
        return "{}";
    }
}

char* get_local_service()
{
    char res_string[4096];
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
    sprintf_s(res_string, 4096, "{\"restapi\":{\"port\":%d,\"listen_address\" : \"%s\"},\"sync\":{\"port\":%d,\"listen_address\" : \"%s\"}}", rest_port, rest_addr, sync_port, sync_addr);

    return res_string;
}



int load_default_instances(const char* file_path)
{
    instances_config = json_object_new_object();
    // save
    dump_json_to_file(instances_config, file_path);
}
/*
void add_instance(const char* ws_id, const char* id, const char* name, const char* path)
{
    struct json_object* add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "name", json_object_new_string(name));
    json_object_object_add(add_new_obj, "path", json_object_new_string(path));
    if (0 != json_object_object_add(instances_config, id, add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }

    dump_json_to_file(instances_config, base_get_instance_path());
}
char* get_instance_path(const char* id)
{
    struct json_object* item;
    if (json_object_object_get_ex(instances_config, id, &item))
    {
        struct json_object* jvalue;
        if (json_object_object_get_ex(item, "path", &jvalue))
        {
            return json_object_get_string(jvalue);
        }
        else
        {
            return "";
        }
    }
    else
    {
        return "";
    }
}

int get_instance_counts()
{
    struct json_object* instances;
    if (json_object_object_get_ex(instances_config, "instances", &instances))
    {
        if (!json_object_is_type(instances, json_type_object))
        {
            return 0;
        }
        int count = 0;
        struct json_object_iterator it = json_object_iter_begin(instances);
        struct json_object_iterator itEnd = json_object_iter_end(instances);
        while (!json_object_iter_equal(&it, &itEnd))
        {
            count++;
            json_object_iter_next(&it);
        }
        return count;
    }
    else
    {
        return 0;
    }
}
*/

int load_default_users(const char* file_path)
{
    //users_config = json_object_new_object();
    users_config = json_object_new_array();
    struct json_object* add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "user", json_object_new_string("admin"));
    json_object_object_add(add_new_obj, "password", json_object_new_string("admin"));
    json_object_object_add(add_new_obj, "roles", json_object_new_string("administrator"));
    if (0 != json_object_array_add(users_config, add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // save
    dump_json_to_file(users_config, file_path);
    json_object_put(add_new_obj);
}

int load_default_neighbors(const char* file_path)
{
    neighbors_config = json_object_new_array();
    // save
    dump_json_to_file(neighbors_config, file_path);
}



int load_file_to_json(struct json_object** json_data, const char* file_path)
{
    FILE* file = fopen(file_path, "r");
    if (!file)
    {
        return CONFIG_LOAD_FILE_ERROR;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    memset(data, 0, length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    *json_data = json_tokener_parse(data);
    free(data);
    return 0;
}

int dump_json_to_file(struct json_object* json_data, const char* file_path)
{
    FILE* file = fopen(file_path, "w");
    if (!file)
    {
        s_log(LOG_ERROR, "open config file: %s error.", file_path);
        return CONFIG_LOAD_FILE_ERROR;
    }
    fwrite(json_object_to_json_string_ext(json_data, JSON_C_TO_STRING_PRETTY), strlen(json_object_to_json_string_ext(json_data, JSON_C_TO_STRING_PRETTY)), 1, file);
    fclose(file);
}


char* get_hostname()
{
    TCHAR compute_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(compute_name) / sizeof(TCHAR);
    char* ch_computer_name = (char*)malloc(sizeof(char) * (MAX_COMPUTERNAME_LENGTH * 2 + 1));
    memset(ch_computer_name, 0, MAX_COMPUTERNAME_LENGTH * 2 + 1);
    if (!GetComputerNameW(compute_name, &size))
    {
        memcpy(ch_computer_name, "unkown", strlen("unkown"));
    }
    else
    {
        TCHARToChar(compute_name, ch_computer_name, MAX_COMPUTERNAME_LENGTH * 2 + 1);
    }
    return ch_computer_name;
}

