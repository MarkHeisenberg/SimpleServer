#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>

typedef struct server server_t;

typedef struct client_info {
    int fd;
    char* ip;
    int port;
} client_info_t;

typedef void (on_connect_cb)(server_t* server, client_info_t* client);

server_t* server_create(const char* ip, int port);

int server_set_timeout(server_t* server, int ms);
int server_run(server_t* server);
int server_stop(server_t* server);

int server_set_on_connect_cb(server_t* server, on_connect_cb* cb);

int server_read(server_t* server, client_info_t* client, char buffer[], int size);
int server_write(server_t* server, client_info_t* client, char buffer[], int size);
size_t server_fwrite(server_t* server, client_info_t* client, FILE *fp, size_t len);
int server_close(server_t* server, client_info_t* client);

void server_destroy(server_t* server);

#endif