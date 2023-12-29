#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

#include "types.h"
#include "headers.h"

int http_request_parse(http_request_t* request, const char* data, size_t length);
void http_request_free(http_request_t* request);
int http_response_init(http_response_t* response);
int http_response_write(http_response_t* response, char* data, size_t length);
void http_response_free(http_response_t* response);
char* http_util_uri_decode(char *str);

#endif // __HTTP_UTILS_H__