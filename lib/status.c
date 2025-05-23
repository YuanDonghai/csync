#include "status.h"

void start_handle_status()
{
    while (1)
    {
        handle_nodes_status();
        handle_instances_status();
        _sleep_or_Sleep(1000);
    }
}

void handle_nodes_status()
{
    struct json_object* nodes_json = get_json_obj(0);
    int array_length = json_object_array_length(nodes_json);
    char uri[4096];
    char* result = NULL;
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* value_obj = json_object_array_get_idx(nodes_json, i);
        struct json_object* node_value_obj;
        if (json_object_object_get_ex(value_obj, "status", &node_value_obj))
        {
            if (0 == strcmp("adding", json_object_get_string(node_value_obj)))
            {
                struct json_object* uri_base;
                if (json_object_object_get_ex(value_obj, "api_url", &uri_base))
                {
                    sprintf_s(uri, 4096, "%s", json_object_get_string(uri_base));
                }
                //
                result = request_restapi_node(uri);
                struct json_object* return_node = json_tokener_parse(result);
                free(result);
                struct json_object* return_node_value;
                if (json_object_object_get_ex(return_node, "name", &return_node_value))
                {
                    json_object_object_add(value_obj, "name", return_node_value);
                }
                if (json_object_object_get_ex(return_node, "id", &return_node_value))
                {
                    if (1 == check_nodes_exist_with_id(json_object_get_string(return_node_value)))
                    {
                        json_object_object_add(value_obj, "status", json_object_new_string("error"));
                        save_json(0);
                        continue;
                    }
                    json_object_object_add(value_obj, "id", return_node_value);
                }
                if (json_object_object_get_ex(return_node, "os_type", &return_node_value))
                {
                    json_object_object_add(value_obj, "os_type", return_node_value);
                }

                result = request_restapi_service(uri);
                return_node = json_tokener_parse(result);
                free(result);
                /*
                {"restapi":{"port":16345,"listen_address" : "0.0.0.0"},"sync":{"port":26345,"listen_address" : "0.0.0.0"}}*/

                if (json_object_object_get_ex(return_node, "sync", &return_node_value))
                {
                    struct json_object* return_node_value_2;
                    if (json_object_object_get_ex(return_node_value, "port", &return_node_value_2))
                    {
                        json_object_object_add(value_obj, "sync_port", return_node_value_2);
                    }

                }
                json_object_object_add(value_obj, "status", json_object_new_string("online"));
                save_json(0);
                // add self to remote
                request_restapi_negotiate_node(uri);
            }
            /*
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
            */
        }
    }
}

void handle_instances_status()
{
    struct json_object* instances_json = get_json_obj(2);

    int array_length = json_object_array_length(instances_json);
    char* result = NULL;
    for (int i = 0; i < array_length; i++)
    {
        struct json_object* value_obj = json_object_array_get_idx(instances_json, i);
        struct json_object* node_value_obj;
        if (json_object_object_get_ex(value_obj, "status", &node_value_obj))
        {
            if (0 == strcmp("adding", json_object_get_string(node_value_obj)))
            {
                struct json_object* node_id;
                char ch_url[4096];
                if (json_object_object_get_ex(value_obj, "peer_node", &node_id))
                {
                    if (0 != get_node_api_url(json_object_get_string(node_id), ch_url))
                    {
                        continue;
                    }
                    request_restapi_negotiate_instance(ch_url, json_object_get_string(value_obj));
                    json_object_object_add(value_obj, "status", json_object_new_string("wait_sure"));
                    save_json(1);
                }
            }

        }
    }
}

char* handle_request(const char* uri, const char* method, const char* header, const char* body)
{
    char* result = NULL;
    CURL* curl;
    CURLcode res;
    //  curl_global_init(CURL_GLOBAL_SSL);
    curl = curl_easy_init();

    if (curl)
    {
        struct ResultString chunk;
        chunk.buff = malloc(1);  // 初始分配1字节
        chunk.size = 0;
        // debug
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
        curl_easy_setopt(curl, CURLOPT_URL, uri);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        res = curl_easy_perform(curl);
        result = (char*)malloc(sizeof(char) * chunk.size);
        memset(result, 0, chunk.size);
        memcpy(result, chunk.buff, chunk.size);
        free(chunk.buff);
    }
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
    {
        return NULL;
    }

    return result;
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct ResultString* res_str = (struct ResultString*)userp;

    char* ptr = realloc(res_str->buff, res_str->size + realsize + 1);
    if (!ptr)
    {
        return 0;
    }
    res_str->buff = ptr;
    memcpy(&(res_str->buff[res_str->size]), contents, realsize);
    res_str->size += realsize;
    res_str->buff[res_str->size] = 0;
    return realsize;
}

char* request_restapi_node(const char* uri)
{
    char server_uri[4096];
    sprintf_s(server_uri, 4096, "%s/api/node", uri);
    return handle_request(server_uri, "GET", "", "");
}

char* request_restapi_service(const char* uri)
{
    char server_uri[4096];
    sprintf_s(server_uri, 4096, "%s/api/service", uri);
    return handle_request(server_uri, "GET", "", "");
}


char* request_restapi_negotiate_node(const char* uri)
{
    char server_uri[4096];
    sprintf_s(server_uri, 4096, "%s/api/negotiate/nodes", uri);
    char res_url[10];
    struct json_object* return_node = json_tokener_parse(get_local_node_info());
    json_object_object_add(return_node, "sync_port", json_object_new_int(base_get_data_listen_port()));
    sprintf_s(res_url, 10, "%d", base_get_restapi_listen_port());
    json_object_object_add(return_node, "api_url", json_object_new_string(res_url));

    return handle_request(server_uri, "POST", "", json_object_get_string(return_node));
}

char* request_restapi_negotiate_instance(const char* uri, const char* body_json)
{
    char server_uri[4096];
    sprintf_s(server_uri, 4096, "%s/api/negotiate/instance", uri);
    char res_url[10];
    struct json_object* return_node = json_tokener_parse(body_json);
    struct json_object* swap_list = json_object_new_object();
    struct json_object* node_value_obj;
    if (json_object_object_get_ex(return_node, "id", &node_value_obj))
    {
        json_object_object_add(swap_list, "peer_iid", json_object_new_string(json_object_get_string(node_value_obj)));
    }
    return_node = json_tokener_parse(get_local_node_info());
    if (json_object_object_get_ex(return_node, "id", &node_value_obj))
    {
        json_object_object_add(swap_list, "peer_node", json_object_new_string(json_object_get_string(node_value_obj)));
    }

    //json_object_object_add(swap_list, "peer_node", get_local_node_id());
    json_object_object_add(swap_list, "id", json_object_new_string(""));
    json_object_object_add(swap_list, "wss_id", json_object_new_string(""));
    json_object_object_add(swap_list, "status", json_object_new_string("wait_sure"));

    handle_request(server_uri, "POST", "", json_object_get_string(swap_list));
    json_object_put(swap_list);
    return "";
}