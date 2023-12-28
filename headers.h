#ifndef __HTTP_HEADERS_H__
#define __HTTP_HEADERS_H__

#include <stdio.h>

#include "types.h"

typedef struct http_headers http_headers_t;

http_headers_t *http_headers_create();
http_headers_t *http_headers_parse(http_request_t *req);
size_t http_headers_snprint(http_headers_t *headers, char *dest, size_t len);
void http_headers_fprintf(http_headers_t *headers, FILE *fp);
int http_headers_set(http_headers_t *headers, char *name, char *value);
char *http_headers_get(http_headers_t *headers, char *name);
void http_headers_remove_header(http_headers_t *headers, char *name);
void http_headers_clear(http_headers_t *headers);
void http_headers_free(http_headers_t *headers);

#endif // __HTTP_HEADERS_H__