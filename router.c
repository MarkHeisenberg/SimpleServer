#include "router.h"

#include <stdlib.h>
#include <string.h>

struct http_middleware {
    struct http_middleware* next; // Next middleware in chain
    http_handler_t* handler; // Handler for this middleware
};

struct http_router_root {
    char* path; // Path for this route
    http_handler_t* handlers[HTTP_METHOD_COUNT]; // Handlers for this route
    struct http_middleware* middleware; // Middleware for this route
    http_router_t* child; // Child router 
    http_router_t* next; // Next route in same route

    http_router_callback_t *on_success;
    void* on_success_data;
    http_router_callback_t *on_failure;
    void* on_failure_data;

};

struct http_router {
    char* path; // Path for this route
    http_handler_t* handlers[HTTP_METHOD_COUNT]; // Handlers for this route
    struct http_middleware* middleware; // Middleware for this route
    http_router_t* child; // Child router 
    http_router_t* next; // Next route in same route
};

http_router_t *http_router_create()
{
    struct http_router_root *router = malloc(sizeof(*router));
    if(router == NULL) return NULL;
    memset(router, 0, sizeof(*router));
    return (http_router_t *)router;
}

static struct http_router *http_router_find_route_in_level(http_router_t *router, char *route)
{
    for(http_router_t *r = router; r != NULL; r = r->next) {
        if(! r->path) continue;
        if(strcmp(r->path, route) == 0) 
            return r;
    }
    return NULL;
}

static struct http_router *http_router_find_route(http_router_t *router, const char *route)
{
    char *path = strdup(route);
    if(path == NULL) return NULL;
    for(char *token = strtok(path, "/") ; token && router; token = strtok(NULL, "/")){
        router = http_router_find_route_in_level(router->child, token);
    }
    free(path);
    return router;
}

static struct http_router *http_router_create_route(http_router_t *router, const char *route)
{
    if(router == NULL) return NULL;
    char *path = strdup(route);
    if(path == NULL) return NULL;
    char *token = strtok(path, "/");
    if(token == NULL) {
        free(path);
        return NULL;
    }
    http_router_t *prev = router;
    http_router_t *r = router->child;
    for( ;token;){
        r = http_router_find_route_in_level(r, token);
        
        if(r == NULL) {
            r = malloc(sizeof(*r));
            if(r == NULL){
                free(path);
                return NULL;
            } 
            memset(r, 0, sizeof(*r));
            r->path = strdup(token);
            if(r->path == NULL){
                free(r);
                return NULL;
            }
            http_router_t *pr = prev->child;
            if(pr == NULL) prev->child = r;
            else{
                while(pr->next != NULL) pr = pr->next;
                pr->next = r;
            }
        }

        prev = r;
        if((token = strtok(NULL, "/")) == NULL) break;
        r = r->child;
    }

    free(path);
    return r;
}

int http_router_set_route(http_router_t *router, const char *route, http_method_t method, http_handler_t *handler)
{
    if(router == NULL) return -1;
    if(route == NULL) return -1;
    if(method >= HTTP_METHOD_COUNT || method < 0) return -1;

    if(strcmp(route, "/") == 0) {
        router->handlers[method] = handler;
        return 0;
    }

    http_router_t *r = http_router_find_route(router, route);
    if(!r) r = http_router_create_route(router, route);
    if(r == NULL) return -1;
    r->handlers[method] = handler;
    return 0;
}

int http_router_remove_route(http_router_t *router, const char *route, http_method_t method)
{
    if(router == NULL) return -1;
    if(route == NULL) return -1; 
    if(method >= HTTP_METHOD_COUNT || method < 0) return -1;
    if(strcmp(route, "/") == 0){
        router->handlers[method] = NULL;
        return 0;
    }

    http_router_t *r = http_router_find_route(router, route);
    if(r == NULL) return -1;
    r->handlers[method] = NULL;
    return 0;
}

int http_router_add_middleware(http_router_t *router, const char *route, http_handler_t *middleware)
{
    if(router == NULL) return -1;
    if(route == NULL) return -1;
    if(middleware == NULL) return -1;
    http_router_t *r = http_router_find_route(router, route);
    if(r == NULL) return -1;
    struct http_middleware *m = malloc(sizeof(*m));
    if(m == NULL) return -1;
    m->handler = middleware;
    m->next = r->middleware;
    r->middleware = m;
    return 0;
}

int http_router_remove_middleware(http_router_t *router, const char *route, http_handler_t *middleware)
{
    if(router == NULL) return -1;
    if(route == NULL) return -1;
    if(middleware == NULL) return -1;
    http_router_t *r = http_router_find_route(router, route);
    if(r == NULL) return -1;
    struct http_middleware *prev = NULL;
    for(struct http_middleware *m = r->middleware; m; m = m->next){
        if(m->handler == middleware){
            struct http_middleware *next = m->next;
            if(prev) prev->next = next;
            if(m == r->middleware) r->middleware = next;
            free(m);
            return 0;
        }
        prev = m;
    }
    return -1;
}

int http_router_on_success(http_router_t *router, http_router_callback_t *callback, void *data)
{
    if(router == NULL) return -1;
    ((struct http_router_root *)(router))->on_success = callback;
    ((struct http_router_root *)(router))->on_success_data = data;
    return 0;
}

int http_router_on_error(http_router_t *router, http_router_callback_t *callback, void *data)
{
    if(router == NULL) return -1;
    ((struct http_router_root *)(router))->on_failure = callback;
    ((struct http_router_root *)(router))->on_failure_data = data;
    return 0;
}

void http_router_handle(http_router_t *router, http_request_t *request, http_response_t *response)
{
    if(router == NULL) return;
    struct http_router_root *root = (struct http_router_root *)router;
    if(root->on_success == NULL) {
        response->status = HTTP_STATUS_CODE__INTERNAL_SERVER_ERROR;
        if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
    }
    
    if(request == NULL){
        response->status = HTTP_STATUS_CODE__BAD_REQUEST;
        if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
        return;
    }

    if(request->path == NULL) {
        response->status = HTTP_STATUS_CODE__BAD_REQUEST;
        if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
        return;
    };
    
    http_router_t *r = http_router_find_route(router, request->path);
    if(r == NULL){
        response->status = HTTP_STATUS_CODE__NOT_FOUND;
        if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
        return;
    }

    if(r->handlers[request->method] != NULL) {
        response->status = HTTP_STATUS_CODE__OK;
        for(struct http_middleware *m = r->middleware; m != NULL; m = m->next){
            const int result = r->middleware->handler(request, response);
            switch(result) {
                case -1: 
                    response->status = HTTP_STATUS_CODE__INTERNAL_SERVER_ERROR;
                    if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
                    return;
                case 0:
                    break;
                case 1:
                    if(root->on_success != NULL) root->on_success(request, response, root->on_success_data);
                    return;
            }
        }
        if(r->handlers[request->method](request, response) == 0) {
            if(root->on_success != NULL) root->on_success(request, response, root->on_success_data);
        } else {
            response->status = HTTP_STATUS_CODE__INTERNAL_SERVER_ERROR;
            if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
        }
    } else {
        response->status = HTTP_STATUS_CODE__NOT_FOUND;
        if(root->on_failure != NULL) root->on_failure(request, response, root->on_failure_data);
    }
}

void http_router_destroy(http_router_t *router)
{
    if(router == NULL) return;
    http_router_t *r = router;
    while(r != NULL){
        if(r->child != NULL) http_router_destroy(r->child);
        
        if(r->middleware != NULL) {
            struct http_middleware *m = r->middleware;
            while(m != NULL){
                struct http_middleware *next = m->next;
                free(m);
                m = next;
            }
        }

        http_router_t *next = r->next;
        if(r->path) free(r->path);
        free(r);
        r = next;
    }
}
