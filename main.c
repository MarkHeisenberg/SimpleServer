#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "router.h"


http_router_callback_t def_callback;

static void print_request(http_request_t *req){
    printf("method: %s\n", http_method_string(req->method));
    printf("path: %s\n", req->path);
}

static void print_response(http_response_t *resp){
    printf("status: %s\n", http_status_string(resp->status));
    printf("headers: %s\n", resp->headers);
    printf("body_length: %ld\n", resp->body_length);
    printf("content_type: %s\n", resp->content_type);
    printf("body: %s\n", resp->body);

}

void def_callback(http_request_t *req, http_response_t *resp, void* data){
    printf("\n==================== REQUEST =====================\n");
    print_request(req);
    printf("===================== RESPONSE =====================\n"); 
    print_response(resp);
    printf("\n==================================================\n");
}

#define CREATE_HANDLER(name) \
http_handler_t name; \
int name(http_request_t *req, http_response_t *resp){ \
    resp->status = 200; \
    resp->content_type = "text/plain"; \
    resp->body = "Handler \""#name"\""; \
    resp->body_length = strlen(resp->body); \
    printf("Handler \""#name"\"\n"); \
    return 0; \
}

CREATE_HANDLER(def_handler)
CREATE_HANDLER(hello_handler)
CREATE_HANDLER(hello2_handler)
CREATE_HANDLER(hello_hello_handler)
CREATE_HANDLER(hello_unexpected_handler)
CREATE_HANDLER(middleware)

int main(int argc, char *argv[]){
    http_router_t *router = http_router_create();
    if(router == NULL){
        printf("router create failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/", HTTP_METHOD_GET, def_handler)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/hello", HTTP_METHOD_GET, hello_handler)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/hello/hello/2", HTTP_METHOD_GET, hello2_handler)
        || http_router_add_middleware(router, "/hello/hello/2", middleware)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/hello/hello", HTTP_METHOD_POST, hello_hello_handler)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/hello/unexpected", HTTP_METHOD_POST, hello_unexpected_handler)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/h/u", HTTP_METHOD_POST, hello_unexpected_handler)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_set_route(router, "/h/unexpected", HTTP_METHOD_POST, hello_unexpected_handler)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_on_success(router, def_callback, NULL)){
        printf("set route failed\n");
        return -1;
    }

    if(http_router_on_error(router, def_callback, NULL)){
        printf("set route failed\n");
        return -1;
    }

    http_request_t req = {
        .path = "/",
        .method = HTTP_METHOD_GET
    };

    http_response_t resp = {0};
    // http_router_handle(router, &req, &resp);
    // req.path = "/hello";
    // http_router_handle(router, &req, &resp);
    // req.path = "/hello/hello";
    // http_router_handle(router, &req, &resp);
    // req.path = "/hello/hello2";
    // http_router_handle(router, &req, &resp);
    req.path = "/hello/hello/2";
    http_router_handle(router, &req, &resp);

    if(http_router_remove_middleware(router, "/hello/hello/2", middleware)){
        printf("remove middleware failed\n");
    } else {
        printf("remove middleware success\n");
        req.path = "/hello/hello/2";
        http_router_handle(router, &req, &resp);
    }

    http_router_destroy(router);
    return 0;
}