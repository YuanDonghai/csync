#include "api.h"
void start_restapi_server(const char* listen_address, int port)
{
    s_log(LOG_DEBUG, "load cert and key.");
    if (0 != get_server_cert_or_key(SERVER_CERT, auth_tls_server_cert))
    {
        s_log(LOG_DEBUG, "load cert error.");
        return;
    }
    if (0 != get_server_cert_or_key(SERVER_KEY, auth_tls_server_key))
    {
        s_log(LOG_DEBUG, "load key error.");
        return;
    }

    if (0 != get_api_server_user("admin", rest_api_pass))
    {
        s_log(LOG_DEBUG, "load admin pass error.");
    }
    char ch_url[256];
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
#if defined(_WIN32) || defined(_WIN64)
    sprintf_s(ch_url, 256, "https://%s:%d", listen_address, port);
    mg_http_listen(&mgr, ch_url, ev_handler, (void*)1);
#elif defined(__linux__)
    sprintf_s(ch_url, 256, "https://%s:%d", listen_address, port);
    mg_http_listen(&mgr, ch_url, ev_handler, (void*)1);
#else
    //others
#endif


   // mg_http_listen(&mgr, ch_url, ev_handler, (void*)1);
    for (;;)
    {
        mg_mgr_poll(&mgr, 1000);
    }
}

int authenticate(struct mg_http_message* hm)
{
    if (!AUTH_ENABLE)
    {
        return 0;
    }
    char user[64], pass[64];
    struct user* u, * result = NULL;
    mg_http_creds(hm, user, sizeof(user), pass, sizeof(pass));

    if (0 == strcmp("admin", user) && 0 == strcmp(pass, rest_api_pass))
    {
        return 0;
    }
    if (0 == strcmp(pass, "1234567890abcdef"))
    {
        return 0;
    }
    return 1;
}

int authenticate_token(const char token)
{

}

// HTTP server event handler function
static void ev_handler(struct mg_connection* c, int ev, void* ev_data)
{

    if (ev == MG_EV_ACCEPT && c->fn_data != NULL)
    {
        struct mg_tls_opts opts = { .cert = mg_str_s(auth_tls_server_cert),
                                    .key = mg_str_s(auth_tls_server_key)
        };
        mg_tls_init(c, &opts);
    }

    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;


        if (0 != authenticate(hm))
        {
            default_path(c, hm, ev_data);
            return;
        }
        ev_handler_path(c, hm, ev_data);
        // mg_http_reply(c, 200, "", "%s", "helloptest");

    }
}

void ev_handler_path(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{

    enum api_controller_path router_index = search_route_index(hm);

    switch (router_index)
    {
    case API_ROOT:
        api_root(c, hm, ev_data);
        break;
    case API_LOGIN:
        handle_login(c, hm, ev_data);
        break;
    case API_NODE:
        api_node(c, hm, ev_data);
        break;
    case API_SERVICE:
        api_service(c, hm, ev_data);
        break;
    case API_NODES:
        api_nodes(c, hm, ev_data);
        break;
    case API_WSS:
        api_wss(c, hm, ev_data);
        break;
    case API_INSTANCES:
        api_instances(c, hm, ev_data);
        break;
    case API_NEGOTIATE_NODES:
        api_negotiate_nodes(c, hm, ev_data);
        break;
    case API_NEGOTIATE_INSTANCE:
        api_negotiate_instance(c, hm, ev_data);
        break;
    case API_INSTANCE_UPDATE:
        api_instance_update(c, hm, ev_data);
        break;
    case INDEX_END:
    default:
        default_path(c, hm, ev_data);
        break;
    }
    /*
    if (mg_match(hm->uri, mg_str("/api/nodes"), NULL))
    {
        if (0 == strncmp("GET", hm->method.buf, 3))
        {
            mg_http_reply(c, 200, "", "%s", api_nodes());
        }
    }
    if (mg_match(hm->uri, mg_str("/api/"), NULL))
    {
        if (0 == strncmp("GET", hm->method.buf, 3))
        {
            mg_http_reply(c, 200, "", "%s", api_nodes());
        }
    }
    else
    {
        struct mg_http_serve_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.root_dir = "web_root";
        mg_http_serve_dir(c, ev_data, &opts);
    }
    */
}

int search_route_index(struct mg_http_message* hm)
{
    int i = 0;
    for (i = 0;i < _API_COUNTS_I;i++)
    {
        if (mg_match(hm->uri, mg_str(api_controller[i]), NULL))
        {
            return i;
        }
    }
    return INDEX_END;
}

int search_method_index(struct mg_http_message* hm)
{
    int i = 0;
    for (i = 0;i < _METHOD_COUNTS_I;i++)
    {
        if (mg_match(hm->method, mg_str(api_methods[i]), NULL))
        {
            return i;
        }
    }
    return METHOD_END;
}


void default_path(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    struct mg_http_serve_opts opts;
    memset(&opts, 0, sizeof(opts));
#if MG_ARCH == MG_ARCH_UNIX || MG_ARCH == MG_ARCH_WIN32
    opts.root_dir = "web_root";  // On workstations, use filesystem
#else
    opts.root_dir = "web_root";  // On embedded, use packed files
    opts.fs = &mg_fs_packed;
#endif
    // opts.root_dir = "web_root";
    // opts.fs = &mg_fs_packed;
    mg_http_serve_dir(c, ev_data, &opts);
}

void api_root(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        mg_http_reply(c, 200, "", "{\"version\":\"%s\"}", VERSION);
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
    s_log(LOG_DEBUG, "api root.");
}
void api_service(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    char res_service[4096];
    switch (method)
    {
    case M_GET:
        memset(res_service, 0, 4096);
        get_local_service(res_service);
        mg_http_reply(c, 200, "", "%s", res_service);
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}
void api_node(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        mg_http_reply(c, 200, "", "%s", get_local_node_info());
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}
void api_nodes(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        mg_http_reply(c, 200, "", "%s", _node_get_nodes());
        break;
    case M_POST:
        mg_http_reply(c, 200, "", "%s", _node_add_nodes(hm->body.buf));
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}

void api_wss(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        mg_http_reply(c, 200, "", "%s", _instance_get_workspace());
        break;
    case M_POST:
        mg_http_reply(c, 200, "", "%s", _instance_add_workspace(hm->body.buf));
        break;

    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}

void api_instances(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        mg_http_reply(c, 200, "", "%s", _instance_get_instance());
        break;
    case M_POST:
        mg_http_reply(c, 200, "", "%s", _instance_add_instance(hm->body.buf));
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}

void api_negotiate_nodes(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        break;
    case M_POST:
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        char client_address[IPV4_ADDRESS_LEN];
        if (getpeername((int)(c->fd), (struct sockaddr*)&client_addr, &client_addr_len) == 0)
        {
            inet_ntop(AF_INET, &client_addr.sin_addr, client_address, IPV4_ADDRESS_LEN);
            //  *port = ntohs(client_addr.sin_port);
             // return 0;
        }
        else
        {
            printf("Failed to get client address: %d\n", WSAGetLastError());
        }
        printf("Client IP: %s\n", client_address);
        s_log(LOG_DEBUG, "client %s : %d", client_address, 0);

        mg_http_reply(c, 200, "", "%s", _node_negotiate_node(hm->body.buf, client_address));
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}

void api_negotiate_instance(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        break;
    case M_POST:
        mg_http_reply(c, 200, "", "%s", _node_negotiate_instance(hm->body.buf));
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}

void api_instance_update(struct mg_connection* c, struct mg_http_message* hm, void* ev_data)
{
    enum api_method method = search_method_index(hm);
    switch (method)
    {
    case M_GET:
        break;
    case M_POST:
        mg_http_reply(c, 200, "", "%s", _node_instance_update(hm->body.buf));
        break;
    default:
        mg_http_reply(c, 405, "", "");
        break;
    }
}

void handle_login(struct mg_connection* c, struct mg_http_message* hm, void* ev_dat)
{
    char cookie[256];
    const char* cookie_name = c->is_tls ? "secure_access_token" : "access_token";
    // const char* cookie_name = "access_token";
    mg_snprintf(cookie, sizeof(cookie),
        "Set-Cookie: %s=%s; Path=/; "
        "%sHttpOnly; SameSite=Lax; Max-Age=%d\r\n",
        cookie_name, "1234567890abcdef",
        c->is_tls ? "Secure; " : "", 3600 * 24);
    mg_http_reply(c, 200, cookie, "{%m:%m}", MG_ESC("user"), MG_ESC("admin"));
}