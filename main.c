#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "types.h"
#include "server.h"
#include "query.h"
#include "headers.h"
#include "parameters.h"
#include "utils.h"
#include "router.h"

static server_t *server;
static http_router_t *router;

on_connect_cb on_connect;

#define BUFFER_SIZE 65353
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
    http_request_t req = {0};
    http_request_parse(&req, buffer, len);
    printf("Method: %s\n", http_method_string(req.method));
    printf("Path: %s\n", req.path);

    http_response_t res;
    if(http_response_init(&res)){
        res.status = HTTP_STATUS_CODE__INTERNAL_SERVER_ERROR;
        len = http_response_write(&res, buffer, BUFFER_SIZE);
        http_request_free(&req);
        server_write(server, client, buffer, len);
        server_close(server, client);
        return;
    }

    http_router_handle(router, &req, &res);

    len = http_response_write(&res, buffer, BUFFER_SIZE);
    http_response_free(&res);
    http_request_free(&req);
    server_write(server, client, buffer, len);
    server_close(server, client);
}

static int running = 1;

void sigint_handler(int sig){
    running = 0;
}

http_handler_t index_cb;
http_handler_t sign_in_cb;
http_handler_t sign_up_cb;
http_router_callback_t error_cb;

int index_cb(http_request_t *req, http_response_t *res){
    FILE *fp = fopen("./www/index.html", "r");
    if(fp == NULL){
        res->status = HTTP_STATUS_CODE__INTERNAL_SERVER_ERROR;
        return -1;
    }
    res->status = HTTP_STATUS_CODE__OK;
    http_headers_set(res->headers, "Content-Type", "text/html");
    fseek(fp, 0, SEEK_END);
    res->body_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    res->body = malloc(res->body_length);
    fread(res->body, 1, res->body_length, fp);
    fclose(fp);
    return 0;
}

int sign_in_cb(http_request_t *req, http_response_t *res){
    http_parameters_t *parameters = http_parameters_parse(req);
    res->status = HTTP_STATUS_CODE__BAD_REQUEST;
    if(parameters == NULL){
        return -1;
    }

    http_parameters_fprintf(parameters, stdout);

    char *message = malloc(1024);
    char *email = malloc(25);
    char *pswd = malloc(25);
    if(http_parameters_get_string(parameters, "email", email, 25) > 0 &&
        http_parameters_get_string(parameters, "pswd", pswd, 25) > 0){
            res->body_length = sprintf(message, 
            "<! DOCTYPE html>"
            "<html><body>"
                "<h1>Hello %s!</h1>"
            "</body></html>",
            email );
        res->status = HTTP_STATUS_CODE__OK;
        http_headers_set(res->headers, "Content-Type", "text/html");
        res->body = message;
        res->body_length = strlen(message);
        printf("Sign in user:%s with pswd: %s\n", email, pswd);
        return 0;
        }
    printf("Error signing in\n");
    free(email);
    free(pswd);
    free(message);
    return -1;
}

int sign_up_cb(http_request_t *req, http_response_t *res){
    res->status = HTTP_STATUS_CODE__BAD_REQUEST;
    http_parameters_t *parameters = http_parameters_parse(req);
    if(parameters == NULL){
        return -1;
    }
    http_parameters_fprintf(parameters, stdout);

    char *message = malloc(1024);
    char *user = malloc(25);
    char *pswd = malloc(25);
    char *email = malloc(25);
    if(http_parameters_get_string(parameters, "user", user, 25) > 0 &&
        http_parameters_get_string(parameters, "pswd", pswd, 25) > 0 &&
        http_parameters_get_string(parameters, "email", email, 25) > 0){
            http_headers_set(res->headers, "Content-Type", "text/html");
            res->body_length = sprintf(message,
            "<! DOCTYPE html>"
            "<html><body>"
                "<h1>You successfully signup</h1>"
                "<p>Name: %s</p>"
                "<p>Password: %s</p>"
                "<p>Email: %s</p>"
            "</body></html>",
            user, pswd, email);
            res->status = HTTP_STATUS_CODE__OK;
            res->body = message;
            printf("Sign up user:%s with pswd: %s\n", email, pswd);
            free(user);
            free(pswd);
            free(email);
            return 0;
    }

    free(user);
    free(pswd);
    free(email);
    free(message);
    printf("Error signing up\n");
    return -1;
}

void error_cb(http_request_t *req, http_response_t *res, void* data){
    res->body = malloc(1024);
    res->body_length = snprintf(res->body, 1024, 
    "<! DOCTYPE html><html><body><h1>ERROROROROROROROROROROROOROR</h1></body></html>");
}

int main(int argc, char *argv[]){
    server = server_create("0.0.0.0", 3002);
    if(server_set_on_connect_cb(server, on_connect)){
        printf("Error setting on_connect callback\n");
        server_destroy(server);
        return 1;
    }

    if((router = http_router_create()) == NULL){
        printf("Error creating router\n");
        server_destroy(server);
        return 1;
    }

    if(http_router_set_route(router, "/", HTTP_METHOD_GET, index_cb) 
         || http_router_set_route(router, "/registartion", HTTP_METHOD_POST, sign_up_cb)
         || http_router_set_route(router, "/login", HTTP_METHOD_POST, sign_in_cb)){
        printf("Error setting routes\n");
        http_router_destroy(router);
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
    http_router_destroy(router);
    return 0;
}