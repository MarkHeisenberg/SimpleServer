#include "parameters.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "list.h"
#include "utils.h"

struct http_parameters {
    char *parameters;
    list_t *list;
};

struct http_parameter_pair {
    char *key;
    char *val;
};

static iterator_t* http_parameters_find(http_parameters_t* p, const char *key){
    for(iterator_t *it = list_begin(p->list); it; it = it->next){
        struct http_parameter_pair *p = (struct http_parameter_pair *)(it->data);
        if(strcmp(p->key, key) == 0){
            return it;
        }
    }
    return NULL;
}

static void http_parameters_parse_parameters(http_parameters_t *p){
    char *param = p->parameters;
    char *pn = NULL;
    for(char *tok = __strtok_r(param, "&", &pn); tok ; 
        tok = __strtok_r(pn, "&", &pn))
    {
        struct http_parameter_pair pair = {
            .key = tok,
            .val = NULL,
        };
        __strtok_r(tok, "=", &pair.val);
        list_push_back(p->list, &pair, sizeof(pair));
    }

    for(iterator_t *it = list_begin(p->list); it != NULL; it = it->next){
        struct http_parameter_pair *pair = (struct http_parameter_pair*)it->data;
        pair->key = http_util_uri_decode(pair->key);
        pair->val = http_util_uri_decode(pair->val);
    }
}

http_parameters_t *http_parameters_parse(http_request_t *request)
{
    if(request == NULL) return NULL;
    if(request->body == NULL) return NULL;
    if(request->body_length == 0) return NULL;
    if(request->method == HTTP_METHOD_GET) return NULL;
    http_parameters_t *p = malloc(sizeof(*p));
    if(p == NULL) return NULL;
    p->parameters = strdup(request->body);
    if(p->parameters == NULL){
        free(p);
        return NULL;
    }

    p->list = list_create();
    if(p->list == NULL){
        free(p->parameters);
        free(p);
        return NULL;
    }
    http_parameters_parse_parameters(p);
    return p;
}

int http_parameters_get_int(http_parameters_t *parameters, const char *name, int *val)
{
    if(parameters == NULL) return -1;
    if(name == NULL) return -1;
    if(val == NULL) return -1;
    iterator_t *it = http_parameters_find(parameters, name);
    if(it == NULL) return -1;
    struct http_parameter_pair *pair = (struct http_parameter_pair*)it->data;
    errno = 0;
    *val = atoi(pair->val);
    return errno;
}

int http_parameters_get_double(http_parameters_t *parameters, const char *name, double *val)
{
    if(parameters == NULL) return -1;
    if(name == NULL) return -1;
    if(val == NULL) return -1;
    iterator_t *it = http_parameters_find(parameters, name);
    if(it == NULL) return -1;
    struct http_parameter_pair *pair = (struct http_parameter_pair*)it->data;
    errno = 0;
    *val = atof(pair->val);
    return errno;
}

int http_parameters_get_string(http_parameters_t *parameters, const char *name, char *val, int len)
{
    if(parameters == NULL) return -1;
    if(name == NULL) return -1;
    if(val == NULL) return -1;
    iterator_t *it = http_parameters_find(parameters, name);
    if(it == NULL) return -1;
    struct http_parameter_pair *pair = (struct http_parameter_pair*)it->data;
    int str_len = strlen(pair->val);
    str_len = str_len >= len - 1 ? len - 1 : str_len;
    strncpy(val, pair->val, str_len);
    val[str_len++] = '\0';
    return str_len;
}

int http_parameters_get_bool(http_parameters_t *parameters, const char *name, int *val)
{
    if(parameters == NULL) return -1;
    if(name == NULL) return -1;
    if(val == NULL) return -1;
    iterator_t *it = http_parameters_find(parameters, name);
    if(it == NULL) return -1;
    struct http_parameter_pair *pair = (struct http_parameter_pair*)it->data;
    *val = 0;
    static char *true_str[] = {
        "true", "True", "TRUE", "1", "yes", "on"
    };
    static int true_count = sizeof(true_str[0]) / sizeof(true_str);
    for(int i = 0; i < true_count; i++){
        if(strcmp(pair->val, true_str[i]) == 0){
            *val = 1;
            break;
        }
    }
    return 0;
}

size_t http_parameters_fprintf(http_parameters_t *parameters, FILE *fp)
{
    if(parameters == NULL) return 0;
    if(fp == NULL) return 0;
    size_t count = 0;
    if(fprintf(fp, "[\n") != 2) return 0;
    for(iterator_t *it = list_begin(parameters->list); it; it = it->next){
        struct http_parameter_pair *pair = (struct http_parameter_pair*)it->data;
        int res = fprintf(fp, "\t[\"%s\"] => \"%s\"\n", pair->key, pair->val);
        if(res < 0) return count;
        count += res;
    }
    fprintf(fp, "]\n");
    return count;
}

void http_parameters_free(http_parameters_t *parameters)
{
    if(parameters == NULL) return;
    if(parameters->parameters != NULL) free(parameters->parameters);
    if(parameters->list != NULL) list_destroy(parameters->list);
    free(parameters);
}
