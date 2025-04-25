#include "node.h"

void set_nodes_json_link(struct json_object* json_p)
{
    neighbors_json = json_p;
}
char* _node_get_nodes()
{
    return json_object_get_string(neighbors_json);
}
char* _node_add_nodes(const char* body_json)
{
    struct json_object* add_new_obj = json_tokener_parse(body_json);
    struct json_object* add_obj_address;
    if (!json_object_object_get_ex(add_new_obj, "address", &add_obj_address))
    {
        return "{\"result\":\"error\",\"error\":\"no address\"}";
    }

    int array_length = json_object_array_length(neighbors_json);
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* element = json_object_array_get_idx(neighbors_json, i);
        struct json_object* pass_obj;
        if (json_object_object_get_ex(element, "address", &pass_obj))
        {
            if (0 == strcmp(json_object_get_string(add_obj_address), json_object_get_string(pass_obj)))
            {
                return "{\"result\":\"error\",\"error\":\"this address exist\"}";
            }        
        }
    }

    json_object_object_add(add_new_obj, "id", json_object_new_string(gen_node_id()));
    

    if (0 == json_object_array_add(neighbors_json, add_new_obj))
    {        
        update_sub_json(1);
    }
    return "{}";
}