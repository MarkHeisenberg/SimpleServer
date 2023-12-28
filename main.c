#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "types.h"
#include "server.h"
#include "query.h"
#include "headers.h"
#include "utils.h"

on_connect_cb on_connect;

#define BUFFER_SIZE 1024
void on_connect(server_t *server, client_info_t* client){
    printf("==========================================================================\n");
    printf("============================ NEW CONNECTION ==============================\n");
    printf("==========================================================================\n");
    printf("Client connected: %s\n", client->ip);
    char buffer[BUFFER_SIZE] = {0};
    int len = server_read(server, client, buffer, BUFFER_SIZE);
    if(len < 0){
        server_close(server, client);
        return;
    }
    printf("Received %d bytes: %s\n", len, buffer);
    printf("==========================================================================\n");
    printf("============================= PARSED VALUES ==============================\n");
    printf("==========================================================================\n");
    http_request_t req = {0};
    http_request_parse(&req, buffer, len);

    char name[25];
    http_query_t *query = http_query_get(&req);
    if(query == NULL){
        printf("Error parsing query\n");
    }else{
        if(http_query_get_string(query, "name", name, 25) > 0){
            printf("Hello  %s!\n", name);
        } else {
            name[0] = '\0';
        }
        printf("Query:\n");
        http_query_fprintf(query, stdout);
        http_query_free(query);
    }

    http_headers_t *headers = http_headers_parse(&req);
    if(headers == NULL){
        printf("Error parsing headers\n");
    } else {
        printf("Headers:\n");
        http_headers_fprintf(headers, stdout);
        http_headers_free(headers);
    }

    http_response_t res;
    if(http_response_init(&res)){
        res.status = HTTP_STATUS_CODE__INTERNAL_SERVER_ERROR;
        len = http_response_write(&res, buffer, BUFFER_SIZE);
        http_request_free(&req);
        printf("==========================================================================\n");
        printf("================================= ANSWER =================================\n");
        printf("==========================================================================\n");
        printf("Response:\n");
        printf("%s\n", buffer);
        server_write(server, client, buffer, len);
        server_close(server, client);
        printf("==========================================================================\n");
        return;
    }

    res.status = HTTP_STATUS_CODE__OK;
    char message[100];
    res.body = message;
    res.body_length = snprintf(message, 100, "Hello %s!", name[0] == '\0' ? "Anonymouse" : name);

    http_headers_set(res.headers, "Content-Type", "text/plain");
    http_headers_set(res.headers, "Content-Language", "en-US");
    http_headers_set(res.headers, "Connection", "keep-alive");
    http_headers_set(res.headers, "Server", "MyServer");
    http_headers_set(res.headers, "Age", "0");
    http_headers_set(res.headers, "Cache-Control", "private");

    printf("==========================================================================\n");
    printf("================================= ANSWER =================================\n");
    printf("==========================================================================\n");
    printf("Response:\n");
    len = http_response_write(&res, buffer, BUFFER_SIZE);
    http_response_free(&res);
    http_request_free(&req);
    printf("%s\n", buffer);
    server_write(server, client, buffer, strlen(buffer));
    server_close(server, client);
    printf("==========================================================================\n");
}

static int running = 1;

void sigint_handler(int sig){
    running = 0;
}


int main(int argc, char *argv[]){
    server_t *server = server_create("0.0.0.0", 3002);
    if(server_set_on_connect_cb(server, on_connect)){
        printf("Error setting on_connect callback\n");
        server_destroy(server);
        return 1;
    }

    if(server_run(server)){
        printf("Error running server\n");
        server_destroy(server);
        return 1;
    }

    server_set_timeout(server, 1000);

    while(running){};

    server_stop(server);
    server_destroy(server);
    return 0;
}