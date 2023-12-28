#ifndef __HTTP_QUERY_H__
#define __HTTP_QUERY_H__

#include <stdbool.h>
#include <stdio.h>

#include "types.h"

typedef struct http_query http_query_t;

http_query_t *http_query_get(http_request_t *req);
int http_query_get_int(http_query_t *query, const char *name, int *val);
int http_query_get_double(http_query_t *query, const char *name, double *val);
int http_query_get_string(http_query_t *query, const char *name, char *val, int len);
int http_query_get_bool(http_query_t *query, const char *name, bool *val);
void http_query_fprintf_query(http_query_t *query, FILE *fp);
void http_query_free(http_query_t *query);

#endif // __HTTP_QUERY_H__