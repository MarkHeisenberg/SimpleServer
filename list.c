#include "list.h"

#include <stdlib.h>
#include <string.h>

struct list_node {
    struct list_node *prev;
    struct list_node *next;
    void *data;
    size_t size;
};

struct list {
    struct list_node *head;
    struct list_node *tail;
};

list_t *list_create()
{
    list_t *list = malloc(sizeof(*list));
    if(list == NULL) return NULL;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

static struct list_node *_list_create_node(void *data, size_t size)
{
    struct list_node *node = malloc(sizeof(*node));
    if(node == NULL) return NULL;
    
    //Allocate memmo for data and copy it
    node->data = calloc(1, size);
    if(node->data == NULL) {
        free(node);
        return NULL;
    }
    memcpy(node->data, data, size);
    node->size = size;
    return node;
}

int list_push_back(list_t *list, void *data, size_t size)
{
    if(list == NULL) return -1;
    //Create new node
    struct list_node *node = _list_create_node(data, size);
    if(node == NULL) return -1;

    //Work with list
    node->prev = list->tail;
    if(list->tail) list->tail->next = node;
    list->tail = node;
    if(list->head == NULL) list->head = node;
    node->next = NULL;
    return 0;
}

int list_push_front(list_t *list, void *data, size_t size)
{
    if(list == NULL) return -1;
    //Create new node
    struct list_node *node = _list_create_node(data, size);
    if(node == NULL) return -1;

    //Work with list
    node->next = list->head;
    if(list->head) list->head->prev = node;
    list->head = node;
    if(list->tail == NULL) list->tail = node;
    node->prev = NULL;
    return 0;
}

int list_pop_back(list_t *list, void *data, size_t size)
{
    if(list == NULL) return -1;
    if(list->tail == NULL) return -1;
    
    //Work with list
    struct list_node *node = list->tail;
    list->tail = node->prev;
    if(list->tail) list->tail->next = NULL;
    if(list->head == node) list->head = NULL;
    
    //Copy data
    const size_t data_size = data
        ? size > node->size ? node->size : size
        : 0;
    
    memcpy(data, node->data, data_size);
    
    //Free node
    free(node->data);
    free(node);
    return data_size;
}

int list_pop_front(list_t *list, void *data, size_t size)
{
    if(list == NULL) return -1;
    if(list->head == NULL) return -1;
    
    //Work with list
    struct list_node *node = list->head;
    list->head = node->next;
    if(list->head) list->head->prev = NULL;
    if(list->tail == node) list->tail = NULL;
    
    //Copy data
    const size_t data_size = data 
        ? size > node->size ? node->size : size
        : 0;
    memcpy(data, node->data, data_size);

    //Free node
    free(node->data);
    free(node);
    return data_size;
}

iterator_t *list_begin(list_t *list)
{
    return list == NULL ? NULL : (iterator_t *)list->head;
}

iterator_t *list_end(list_t *list)
{
    return list == NULL ? NULL : (iterator_t *)list->tail;
}

int list_remove(list_t *list, iterator_t *it)
{
    if(list == NULL) return -1;
    if(it == NULL) return -1;
    struct list_node *node = (struct list_node *)it;

    //Work with list
    if(node->prev) node->prev->next = node->next;
    if(node->next) node->next->prev = node->prev;
    if(list->head == node) list->head = node->next;
    if(list->tail == node) list->tail = node->prev;

    //Free node
    free(node->data);
    free(node);
    return 0;
}

int list_insert_after(list_t *list, iterator_t *it, void *data, size_t size)
{
    if(list == NULL) return -1;
    if(it == NULL) return -1;
    struct list_node *node = (struct list_node *)it;
    struct list_node *new_node = _list_create_node(data, size);
    if(new_node == NULL) return -1;

    //Work with list
    new_node->prev = node;
    new_node->next = node->next;
    if(node->next) node->next->prev = new_node;
    node->next = new_node;
    if(list->tail == node) list->tail = new_node;
    return 0;
}

int list_insert_before(list_t *list, iterator_t *it, void *data, size_t size)
{
    if(list == NULL) return -1;
    if(it == NULL) return -1;
    struct list_node *node = (struct list_node *)it;
    struct list_node *new_node = _list_create_node(data, size);
    if(new_node == NULL) return -1;

    //Work with list
    new_node->next = node;
    new_node->prev = node->prev;
    if(node->prev) node->prev->next = new_node;
    node->prev = new_node;
    if(list->head == node) list->head = new_node;
    return 0;
}

void list_destroy(list_t *list)
{
    if(list == NULL) return;
    while(list->head) {
        list_pop_front(list, NULL, 0);
    }
    free(list);
}
