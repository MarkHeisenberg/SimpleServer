#ifndef __HTTP_DIRECTORY_H__
#define __HTTP_DIRECTORY_H__

#include <stdlib.h>
#include <stdio.h>

typedef struct http_directory http_directory_t;

http_directory_t* http_directory_init();
int http_directory_add_file(http_directory_t* dir, const char* path, const char* filename);
int http_directory_add_dir(http_directory_t* dir, const char* path, const char* dirname);
int http_directory_parse(http_directory_t* dir, const char* filepath);
const char* http_directory_get_content(http_directory_t* dir, const char* path);
size_t http_directory_get_content_size(http_directory_t* dir, const char* path);
size_t http_directory_fprintf(http_directory_t* dir, FILE* fp, const char* path);
size_t http_directory_snprintf(http_directory_t* dir, char* buf, size_t size, const char* path);
void http_directory_destroy();

#endif //__HTTP_DIRECTORY_H__