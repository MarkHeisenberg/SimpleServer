#include "router.h"

#include <stdlib.h>
#include <string.h>

struct http_router {
    char* path;
    http_handler_t* handler;
    http_router_t* next;
    http_router_t* child;
};

http_router_t *http_router_create()
{
    http_router_t *router = malloc(sizeof(*router));
    if(router == NULL) return NULL;
    memset(router, 0, sizeof(*router));
    return router;
}
