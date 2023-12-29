#ifndef __HTTP_PARAMETERS_H__
#define __HTTP_PARAMETERS_H__

#include <stdio.h>

#include "types.h"

typedef struct http_parameters http_parameters_t;

http_parameters_t *http_parameters_parse(http_request_t *request);
int http_parameters_get_int(http_parameters_t *parameters, const char *name, int *val);
int http_parameters_get_double(http_parameters_t *parameters, const char *name, double *val);
int http_parameters_get_string(http_parameters_t *parameters, const char *name, char *val, int len);
int http_parameters_get_bool(http_parameters_t *parameters, const char *name, int *val);
size_t http_parameters_fprintf(http_parameters_t *parameters, FILE *fp);
void http_parameters_free(http_parameters_t *parameters);

#endif // __HTTP_PARAMETERS_H__