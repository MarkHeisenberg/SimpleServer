#ifndef __HTTP_LIST_H__
#define __HTTP_LIST_H__

typedef struct iterator {
    struct iterator* prev;
    struct iterator* next;
    void* data;
    size_t size;
} iterator_t;

typedef struct list list_t;

list_t *list_create();
int list_push_back(list_t *list, void *data, size_t size);
int list_push_front(list_t *list, void *data, size_t size);
int list_pop_back(list_t *list, void *data, size_t size);
int list_pop_front(list_t *list, void *data, size_t size);
iterator_t *list_begin(list_t *list);
iterator_t *list_end(list_t *list);
int list_remove(list_t *list, iterator_t *it);
int list_insert_after(list_t *list, iterator_t *it, void *data, size_t size);
int list_insert_before(list_t *list, iterator_t *it, void *data, size_t size);
void list_destroy(list_t *list);

#endif