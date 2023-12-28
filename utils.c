#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

http_method_t http_method_from_string(const char *string){
    if(string == NULL) return HTTP_METHOD_COUNT;
    for(int i = 0; i < HTTP_METHOD_COUNT; i++){
        if(strcmp(string, http_method_string((http_method_t)(i))) == 0) 
            return (http_method_t)(i);
    }
    return HTTP_METHOD_COUNT;
}

int http_request_parse(http_request_t *request, const char *data, size_t length)
{
    if (data == NULL || length < 1) return -1;
    const char *end = data + length;
    request->data = data;
    request->data_length = length;

    //Set method
    request->method = http_method_from_string(data);
    const char *p_start = request->data + strlen(http_method_string(request->method)) + 1;

    //Set body
    request->body = NULL;
    request->body_length = 0;
    for(const char *b_start = p_start; b_start != end ; b_start++){
        if(strncmp(b_start, "\r\n\r\n", 4) == 0 ){
            request->body = b_start + 4;
            request->body_length = end - request->body;
            break;
        } else if (strncmp(b_start, "\n\n", 2) == 0){
            request->body = b_start + 2;
            request->body_length = end - request->body;
            break;
        }
    }

    //Set path
    const char *p_end = p_start;
    while(*p_end != ' ' && *p_end != '?' && *p_end != '\n' && *p_end != '\r' ){
        if(*p_end == '\0' || p_end == end) return -1;
        p_end++;
    } 
    request->path = (char*)malloc(p_end - p_start + 1);
    memcpy(request->path, p_start, p_end - p_start);
    request->path[p_end - p_start] = '\0';
    request->path = NULL;
    return 0;
}

void http_request_free(http_request_t *request)
{
    if(request == NULL) return;
    if(request->path != NULL) free(request->path);
    request->path = NULL;
    request->data = NULL;
    request->body = NULL;
    request->body_length = 0;
    request->data_length = 0;
    request->method = HTTP_METHOD_COUNT;
}

int http_response_write(http_response_t *response, char *data, size_t length)
{
    if (data == NULL || length < 1) return -1;
    char *body_len_str = http_headers_get(response->headers, "Content-Length");
    if(body_len_str == NULL){
        if(response->body_length < 0) return -1;
        body_len_str = (char*)malloc(20);
        if(body_len_str == NULL) return -1;
        snprintf(body_len_str, 20, "%d", response->body_length);
        if(http_headers_set(response->headers, "Content-Length", body_len_str)){
            free(body_len_str);
            return -1;
        }
        free(body_len_str);
    }
    int cursor = 0;

    int res = snprintf(data + cursor, length, "HTTP/2.0 %d %s\r\n", response->status, http_status_string(response->status));
    if(res < 0) return -1;
    cursor += res;

    res = http_headers_snprint(response->headers, data + cursor, length - cursor);
    if(res < 0) return -1;
    cursor += res;

    res = snprintf(data + cursor, length - cursor, "\r\n");
    if(res < 0) return -1;
    cursor += res;

    res = snprintf(data + cursor, length - cursor, "%s", response->body);
    if(res < 0) return -1;
    cursor += res;

    return 0;
}

void http_response_free(http_response_t *response)
{
    if(response == NULL) return;
}
