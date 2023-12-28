#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "types.h"
#include "server.h"
#include "query.h"

on_connect_cb on_connect;

#define BUFFER_SIZE 1024
void on_connect(server_t *server, client_info_t* client){
    printf("Client connected: %s\n", client->ip);
    char buffer[BUFFER_SIZE] = {0};
    server_read(server, client, buffer, BUFFER_SIZE);
    char *pbuf = buffer + 1;
    while(*pbuf != '\0' || pbuf <= &buffer[BUFFER_SIZE]) {
        if(*pbuf == '\n' && *(pbuf-1) == '\n') 
            *pbuf = '\0';
        pbuf++;
    }
    http_request_t req = {
        .headers = buffer,
        .body = ++pbuf
    };
    printf("Headers: \n%s\n", req.headers);
    http_query_t *query = http_query_get(&req);
    if(query == NULL){
        printf("Error parsing query\n");
    }else{
        char name[25];
        if(http_query_get_string(query, "name", name, 25) > 0){
            printf("Hello  %s!\n", name);
        } else {
            printf("Hello anononymous!\n");
        }
        http_query_fprintf_query(query, stdout);
    }
    server_write(server, client, buffer, strlen(buffer));
    server_close(server, client);
}

static int running = 1;

void sigint_handler(int sig){
    running = 0;
}


int main(int argc, char *argv[]){
    server_t *server = server_create("0.0.0.0", 3002);
    if(server_set_on_connect_cb(server, on_connect)){
        printf("Error setting on_connect callback\n");
        return 1;
    }

    if(server_run(server)){
        printf("Error running server\n");
        return 1;
    }

    while(running){};

    server_stop(server);
    server_destroy(server);
    return 0;
}