#include <stdio.h>
#include <unistd.h>

#include "server.h"
#include "http/types.h"
#include "http/list.h"

on_connect_cb on_connect;

void on_connect(server_t *server, client_info_t *client){
    printf("Client connected: %s:%d\n", client->ip, client->port);
    char buff[1024] = {0};
    if(server_read(server, client, buff, 1024)){
        printf("Client sent: \n%s\n", buff);
        server_write(server, client, "Hello from server!", 19);
    }
    server_close(server, client);
}

int main(int argc, char *argv[]){
    list_t *list = list_create();
    if(list == NULL){
        printf("Error creating list\n");
        return -1;
    }

    if(list_push_back(list, "Hello", 6)
        || list_push_back(list, "World", 6)
        || list_push_back(list, "!", 1)
        || list_push_front(list, " <> ", 5)){
            printf("Error adding to list\n");
            list_destroy(list);
            return -1;
        }

    printf("List: ");
    for(iterator_t *it = list_begin(list); it; it = it->next){
        printf("%s ", (char *)it->data);
    }
    printf("\n");

    printf("List by element: \n");
    for(iterator_t *it = list_begin(list); it; it = it->next){
        printf("\t data: %s\t size: %ld\n", (char *)it->data, it->size);
    }
    printf("\n");
    list_destroy(list);
}