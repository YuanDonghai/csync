#include "user.h"

int load_users_config(const char* file_path)
{
    s_log(LOG_INFO, "loading users config %s.", file_path);
    if (0 != load_file_to_json(&user_json, file_path))
    {
        if (0 != load_default_users(file_path))
        {
            s_log(LOG_ERROR, "can not load users metadata.");
            return -1;
        }
    }
    return 0;
}

int load_default_users(const char* file_path)
{
    user_json = json_object_new_array();
    struct json_object* add_new_obj = json_object_new_object();
    json_object_object_add(add_new_obj, "user", json_object_new_string("admin"));
    json_object_object_add(add_new_obj, "password", json_object_new_string("admin"));
    json_object_object_add(add_new_obj, "roles", json_object_new_string("administrator"));
    if (0 != json_object_array_add(user_json, add_new_obj))
    {
        s_log(LOG_ERROR, "json add error");
    }
    // save
    dump_json_to_file(user_json, file_path);
    json_object_put(add_new_obj);
    return 0;
}

int get_api_server_user(char* user, char* pass)
{
    if (!json_object_is_type(user_json, json_type_array))
    {
        s_log(LOG_ERROR, "the user config is not an array.");
        return 1;
    }
    int array_length = json_object_array_length(user_json);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(user_json, i);
        struct json_object* pass_obj;
        if (json_object_object_get_ex(element, "user", &pass_obj))
        {
            if (0 == strcmp(user, json_object_get_string(pass_obj)))
            {
                if (json_object_object_get_ex(element, "password", &pass_obj))
                {
                    sprintf_s(pass, 64, "%s", json_object_get_string(pass_obj));
                    return 0;
                }
            }
        }
    }
    return 1;
}
