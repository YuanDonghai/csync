#include "status.h"

void start_handle_status()
{
    while (1)
    {
        handle_nodes_status();
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
                    json_object_object_add(value_obj, "id", return_node_value);
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