#include "query.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "list.h"

#define PARAMETER_DELIMETER "&"


struct http_query {
    list_t *list;
    char *query;
};

struct http_query_param {
    char *key;
    char *val;
};

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

static iterator_t *http_query_find_parameter(http_query_t *q, const char *key){
    if(q == NULL || key == NULL) return NULL;
    for(iterator_t *it = list_begin(q->list); it != NULL; it = it->next){
        struct http_query_param *p = (struct http_query_param*)it->data;
        if(strcmp(p->key, key) == 0) return it;
    }

    return NULL;
}

static void shift_str(char *str, int shift){
    const int len = strlen(str) - shift;
    for(int i = 0; i < len; i++){
        str[i] = str[i + shift];
    }
    str[len] = '\0';
}

static char* http_uri_decode(char *str){
    char hex[3] = {0, [2] = '\0'};
    int len = strlen(str);
    for(int i = 0; i < len; i++){
        if(str[i] == '%'){
            hex[0] = str[i + 1];
            hex[1] = str[i + 2];
            str[i] = (char)strtol(hex, NULL , 16);
            shift_str(&str[i+1], 2);
            len -= 2;
        }
    }
    return str;
}

static int http_query_add_param(http_query_t *q, struct http_query_param *p){
    if(q == NULL || p == NULL || q->list == NULL) return -1;
    iterator_t *it = http_query_find_parameter(q, p->key);
    if(it == NULL){
        return list_push_back(q->list, p, sizeof(*p));
    }
    
    return -1;
}

static int http_query_parse_query(http_query_t *q){
    q->list = list_create();
    if(q->list == NULL) return -1;
    char *query = q->query;
    char *pn = NULL;
    for(char *tok = __strtok_r(query, PARAMETER_DELIMETER, &pn); tok ; 
        tok = __strtok_r(pn, PARAMETER_DELIMETER, &pn))
    {
        struct http_query_param param = {
            .key = tok,
            .val = NULL,
        };
        __strtok_r(tok, "=", &param.val);
        if(http_query_add_param(q, &param)) {
            list_destroy(q->list);
            return -1;
        }
    }
    for(iterator_t *it = list_begin(q->list); it != NULL; it = it->next){
        struct http_query_param *p = (struct http_query_param*)it->data;
        p->key = http_uri_decode(p->key);
        p->val = http_uri_decode(p->val);
    }
    return 0;
}

http_query_t *http_query_get(http_request_t *req)
{
    char *query = http_query_find_query(req);
    if(query == NULL) return NULL;
    
    http_query_t *q = malloc(sizeof(http_query_t));
    if(q == NULL) {
        free(query);
        return NULL;
    }
    q->query = query;
    
    if(http_query_parse_query(q)){
        free(query);
        free(q);
        return NULL;
    }
    return q;
}

int http_query_get_int(http_query_t *query, const char *name, int *val)
{
    if(query == NULL || name == NULL || val == NULL) return -1;
    iterator_t *it = http_query_find_parameter(query, name);
    if(it == NULL) return -1;
    struct http_query_param *p = (struct http_query_param*)it->data;
    errno = 0;
    *val = atoi(p->val);
    if(errno != 0) return -1;
    return 1;
}

int http_query_get_double(http_query_t *query, const char *name, double *val)
{
    if(query == NULL || name == NULL || val == NULL) return -1;
    iterator_t *it = http_query_find_parameter(query, name);
    if(it == NULL) return -1;
    struct http_query_param *p = (struct http_query_param*)it->data;
    errno = 0;
    *val = atof(p->val);
    if(errno != 0) return -1;
    return 1;
}

int http_query_get_string(http_query_t *query, const char *name, char *val, int len)
{
    if(query == NULL || name == NULL || val == NULL) return -1;
    iterator_t *it = http_query_find_parameter(query, name);
    if(it == NULL) return -1;
    struct http_query_param *p = (struct http_query_param*)it->data;
    int str_len = strlen(p->val);
    if(str_len > len) str_len = len;
    strncpy(val, p->val, str_len);
    val[str_len] = '\0';
    return str_len;
}

int http_query_get_bool(http_query_t *query, const char *name, bool *val)
{
    if(query == NULL || name == NULL || val == NULL) return -1;
    iterator_t *it = http_query_find_parameter(query, name);
    if(it == NULL) return -1;
    struct http_query_param *p = (struct http_query_param*)it->data;
    if(strcmp(p->val, "true") == 0
     || strcmp(p->val, "True") == 0) {
        *val = true;
    } else *val = false;
    return 1;
}

void http_query_fprintf_query(http_query_t *query, FILE *fp)
{
    if(query == NULL || fp  == NULL) return;
    fprintf(fp, "[\n");
    for(iterator_t *it = list_begin(query->list); it != NULL; it = it->next){
        struct http_query_param *p = (struct http_query_param*)it->data;
        fprintf(fp, "\t\"%s\" => \"%s\",\n", p->key, p->val);
    }
    fprintf(fp, "]\n");
}

void http_query_free(http_query_t *query)
{
    if(query) {
        list_destroy(query->list);
        free(query->query);
        free(query);
    }
}
