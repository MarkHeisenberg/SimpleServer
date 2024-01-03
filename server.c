#include "server.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>

struct server {
    int fd;
    int port;
    char *ip;
    volatile bool running;
    pthread_t tid;
    on_connect_cb *on_connect;
};

server_t *server_create(const char *ip, int port)
{
    struct server *s = malloc(sizeof(*s));
    if(!s) return NULL;
    s->ip = strdup(ip);
    if(!s->ip){
        free(s);
        return NULL;
    }
    s->port = port;
    s->fd = -1;
    s->on_connect = NULL;
    s->tid = 0;
    s->running = false;
    return s;
}

static void close_socket(int fd){
    if(fd){
        static int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        close(fd);
    }
}

static void *server_run_thread(void *arg)
{
    struct server *s = arg;
    s->running = true;
    while(s->running){
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int fd = accept(s->fd, (struct sockaddr *)&addr, &len);
        if(fd < 0) continue;
        client_info_t client = {
            .ip = inet_ntoa(addr.sin_addr),
            .port = ntohs(addr.sin_port),
            .fd = fd };
        if(s->on_connect){
            s->on_connect(s, &client);
        }
    }
    return NULL;
}

int server_set_timeout(server_t *server, int ms)
{
    if(!server || !server->running) return -1;
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return setsockopt(server->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int server_run(server_t *server)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) return -1;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);
    addr.sin_addr.s_addr = inet_addr(server->ip);
    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        close_socket(fd);
        return -1;
    }

    if(listen(fd, 5)){
        close_socket(fd);
        return -1;
    }

    server->fd = fd;
    if(pthread_create(&server->tid, NULL, server_run_thread, server) < 0)
    {
        close_socket(fd);
        return -1;
    }

    return 0;
}

int server_stop(server_t *server)
{
    if(!server) return -1;
    if(server->running == false) return 0;
    server->running = false;
    close_socket(server->fd);
    pthread_join(server->tid, NULL);
    return 0;
}

int server_set_on_connect_cb(server_t *server, on_connect_cb *cb)
{
    if(!server) return -1;
    server->on_connect = cb;
    return 0;
}

int server_read(server_t *server, client_info_t *client, char buffer[], int size)
{
    if(!server || !client) return -1;
    return read(client->fd, buffer, size); 
}

int server_write(server_t *server, client_info_t *client, char buffer[], int size)
{
    if(!server || !client) return -1;
    return write(client->fd, buffer, size);
}

size_t server_fwrite(server_t *server, client_info_t *client, FILE *fp, size_t len)
{
    if(!server || !client) return -1;
    char buffer[1024] = {0};
    size_t total = 0;
    while(len > 0){
        int n = fread(buffer, 1, 1024, fp);
        if(n <= 0) break;
        int m = server_write(server, client, buffer, n);
        if(m <= 0) break;
        total += m;
        len -= m;
        if(len <= 0 || m < n) break;
    }
    return total;
}

int server_close(server_t *server, client_info_t *client)
{
    if(!server || !client) return -1;
    close(client->fd);
    return 0;
}

void server_destroy(server_t *server)
{
    server_stop(server);
    free(server->ip);
    free(server);
}
