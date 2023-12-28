#include "headers.h"

#include <string.h>

#include "list.h"

struct http_headers {
    list_t *list;
    char* raw;
};

struct http_header_pair {
    char *key;
    char *value;
};

static struct http_header_pair *http_header_pair_create(char* key, char* value){
    struct http_header_pair *pair = (struct http_header_pair *)malloc(sizeof(*pair));
    if(pair == NULL) return NULL;
    
    pair->key = strdup(key);
    if(pair->key == NULL) {
        free(pair);
        return NULL;
    }

    pair->value = strdup(value);
    if(pair->value == NULL) {
        free(pair->key);
        free(pair);
        return NULL;
    }

    return pair;
}

static void http_header_pair_destroy_data(struct http_header_pair *pair){
    if(pair == NULL) return;
    free(pair->key);
    free(pair->value);
}

http_headers_t *http_headers_create()
{
    http_headers_t *headers = malloc(sizeof(*headers));
    if(headers == NULL) return NULL;

    headers->list = list_create();
    if(headers->list == NULL) {
        free(headers);
        return NULL;
    }

    headers->raw = NULL;
    return headers;
}

static iterator_t *http_headers_find_header(http_headers_t *headers, char *name)
{
    for(iterator_t *it = list_begin(headers->list); it; it = it->next){
        struct http_header_pair *pair = (struct http_header_pair *)it->data;
        if(strcmp(pair->key, name) == 0) return it;
    }
    return NULL;
}

static int http_headers_parse_raw(http_headers_t *headers){
    if(headers == NULL) return -1;
    const char *end = headers->raw + strlen(headers->raw);
    struct http_header_pair pair;
    char has_pair_on_line = 0;
    for(char *p = headers->raw, *q = headers->raw; q < end; q++) {
        if(*q == ':' && !has_pair_on_line) {
            pair.key = p;
            pair.value = q + 2;
            *q = '\0';
            has_pair_on_line = 1;
        }
        if(*q == '\n' && has_pair_on_line) {
            if(q > headers->raw && *(q - 1) == '\r'){
                *(q - 1) = '\0';
            } else {
                *q = '\0';
            }
            p = q + 1;
            has_pair_on_line = 0;
            
            if(http_headers_set(headers, pair.key, pair.value) != 0){
                http_headers_clear(headers);
                return -1;
            }
        }
    }
    return 0;
}


int http_headers_set(http_headers_t *headers, char *name, char *value)
{
    iterator_t *it = http_headers_find_header(headers, name);
    if(it != NULL){
        struct http_header_pair *pair = (struct http_header_pair *)it->data;
        char *new_value = strdup(value);
        if(new_value == NULL) return -1;
        free(pair->value);
        pair->value = new_value;
    } else {
        struct http_header_pair *pair = http_header_pair_create(name, value);
        if(pair == NULL) return -1;
        if(list_push_back(headers->list, pair, sizeof(*pair))) {
            http_header_pair_destroy_data(pair);
            free(pair);
            return -1;
        }
        free(pair);
    }
    return 0;
}

char *http_headers_get_header(http_headers_t *headers, char *name)
{
    iterator_t *it = http_headers_find_header(headers, name);
    return it ? ((struct http_header_pair *)it->data)->value : NULL;
}

void http_headers_remove_header(http_headers_t *headers, char *name)
{
    iterator_t *it = http_headers_find_header(headers, name);
    if(it != NULL){
        struct http_header_pair *pair = (struct http_header_pair *)it->data;
        http_header_pair_destroy_data(pair);
        list_remove(headers->list, it);
    }
}

http_headers_t *http_headers_parse(http_request_t *req)
{
    char *h_start = req->data;
    char *end = (req->data + req->data_length);
    if(h_start == NULL) return NULL;
    http_headers_t *headers = http_headers_create();
    if(headers == NULL) return NULL;
    
    for(;h_start <= end; h_start++) {
        if(*h_start == '\n') {h_start++; break;}
    }
    if(h_start == end) return headers;
    
    char *h_end = h_start;
    for(;h_end <= end; h_end++) {
        if(( h_end + 1 < end && *h_end == '\n' && *(h_end + 1) == '\n')
            || ( h_end + 2 < end && *h_end == '\n' && *(h_end + 1) == '\r' && *(h_end + 2) == '\n')
        ) {
            break;
        }
    }

    if(h_end == end) return headers;
    headers->raw = (char*)malloc(h_end - h_start + 1);
    if(headers->raw == NULL) {
        http_headers_free(headers);
        return NULL;
    };

    memcpy(headers->raw, h_start, h_end - h_start + 1);
    headers->raw[h_end - h_start] = '\0';

    if(http_headers_parse_raw(headers) != 0) {
        http_headers_free(headers);
        return NULL;
    }

    return headers;
}

size_t http_headers_snprint(http_headers_t *headers, char *dest, size_t len)
{
    size_t size = 0;
    for(iterator_t *it = list_begin(headers->list); it; it = it->next){
        struct http_header_pair *pair = (struct http_header_pair *)it->data;
        size += snprintf(dest + size, len - size,"%s: %s\r\n", pair->key, pair->value);
        if(size >= len) break;
    }
    return size;
}

void http_headers_fprintf(http_headers_t *headers, FILE *fp)
{
    fprintf(fp, "[\n");
    for(iterator_t *it = list_begin(headers->list); it; it = it->next){
        struct http_header_pair *pair = (struct http_header_pair *)it->data;
        fprintf(fp, "\t[\"%s\"] => \"%s\"\n", pair->key, pair->value);
    }
    fprintf(fp, "]\n");
}

void http_headers_clear(http_headers_t *headers)
{   
    if(headers == NULL || headers->list == NULL) return;
    for(iterator_t *it = list_begin(headers->list); it; ){
        http_header_pair_destroy_data((struct http_header_pair *)it->data);
        iterator_t *next = it->next;
        list_remove(headers->list, it);
        it = next;
    }
}

void http_headers_free(http_headers_t *headers)
{
    if(headers == NULL) return;
    http_headers_clear(headers);
    list_destroy(headers->list);
    free(headers->raw);
    free(headers);
}
