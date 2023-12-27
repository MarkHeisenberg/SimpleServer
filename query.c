#include "query.h"

#include <string.h>

#include <stdio.h>

#include "list.h"

static char* http_query_find_query(http_request_t *req)
{
    if(req == NULL) return NULL;
    if(req->headers == NULL) return NULL;
    const int len = strlen(req->headers);
    if(len == 0) return NULL;
    const char *headers_end = req->headers + len;
    char *start = NULL, *end = NULL;
    start = end = req->headers;
    while(end != NULL && *end != '\0' && *end != '\n' && end <= headers_end){
        end++; //pointer to the end of the line
    } 
    if(end == NULL || end == start) return NULL;
    while(end-- != start) if(*end == ' ') break; //find the ' ' as end of parameters string
    if(end == start) return NULL;
    while (start != end) if(*start++ == '?') break; //find the '?' as start of parameters string
    if(start == end) return NULL;
    char *query = malloc(end - start + 1);
    if(query == NULL) return NULL;
    memcpy(query, start, end - start);
    query[end - start] = '\0';
    return query;
}

http_query_t *http_query_get(http_request_t *req)
{
    char *query = http_query_find_query(req);
    if(query == NULL) return NULL;
    printf("query: %s\n", query);
    return NULL;
}