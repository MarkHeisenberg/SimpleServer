#ifndef __HTTP_ROUTER_H__
#define __HTTP_ROUTER_H__

#include "types.h"

typedef struct http_router http_router_t;
typedef void(http_router_callback_t)(http_request_t* request, http_response_t* response, void* data);

http_router_t* http_router_create();
int http_router_set_route(http_router_t* router, const char* route, http_method_t method, http_handler_t* handler);
int http_router_remove_route(http_router_t* router, const char* route, http_method_t method);
int http_router_add_middleware(http_router_t* router, const char* route, http_handler_t* middleware);
int http_router_remove_middleware(http_router_t* router, const char* route, http_handler_t* middleware);
int http_router_on_success(http_router_t* router, http_router_callback_t* callback, void* data);
int http_router_on_error(http_router_t* router, http_router_callback_t* callback, void* data);
void http_router_handle(http_router_t* router, http_request_t* request, http_response_t *response);
void http_router_destroy(http_router_t* router);


#endif // __HTTP_ROUTER_H__