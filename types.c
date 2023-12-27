#include "types.h"

#include <stdlib.h>

const char *http_status_string(http_status_t status)
{
    static const char *isc[] = {
        [0] = "Continue",
        [1] = "Switching ptotocols",
        [2] = "Processing",
        [3] = "Early Hints"};

    static const char *ssc[] = {
        [0] = "OK",
        [1] = "Created",
        [2] = "Accepted",
        [3] = "Non-Authoritative Information",
        [4] = "No Content",
        [5] = "Reset Content",
        [6] = "Partial Content",
        [7] = "Multi-Status",
        [8] = "Already Reported",
        [26] = "IM Used"};

    static const char *rsc[] = {
        [0] = "Multiple Choices",
        [1] = "Moved Permanently",
        [2] = "Found",
        [3] = "See Other",
        [4] = "Not Modified",
        [5] = "Use Proxy",
        [6] = "Unused",
        [7] = "Temporary Redirect",
        [8] = "Permanent Redirect"};

    static const char *cesc[] = {
        [0] = "Bad Request",
        [1] = "Unauthorized",
        [2] = "Payment Required",
        [3] = "Forbidden",
        [4] = "Not Found",
        [5] = "Method Not Allowed",
        [6] = "Not Acceptable",
        [8] = "Request Timeout",
        [9] = "Conflict",
        [10] = "Gone",
        [11] = "Length Required",
        [12] = "Precondition Failed",
        [13] = "Payload Too Large",
        [14] = "URIToo Long",
        [15] = "Unsupported Media Type",
        [16] = "Range Not Satisfiable",
        [17] = "Expectation Failed",
        [18] = "Server need to drink some coffee",
        [21] = "Misdirected Request",
        [22] = "Unprocessable Entity",
        [23] = "Locked",
        [24] = "Failed Dependency",
        [25] = "Too Early",
        [26] = "Upgrade Required",
        [28] = "Precondition Required",
        [29] = "Too Many Requests",
        [31] = "Request Header Fields Too Large",
        [51] = "Unavailable For Legal Reasons",
    };

    static const char *sesc[] = {
        [0] = "Internal Server Error",
        [1] = "Not Implemented",
        [2] = "Bad Gateway",
        [3] = "Service Unavailable",
        [4] = "Gateway Timeout",
        [5] = "HTTP Version Not Supported",
        [6] = "Variant Also Negotiates",
        [7] = "Insufficient Storage",
        [8] = "Loop Detected",
        [10] = "Not Extended",
        [11] = "Network Authentication Required"};

    if (status < 100) return NULL;
    if (status < 200) {
        const int status_count = sizeof(isc) / sizeof(isc[0]);
        const unsigned int requested_status = status - 100;
        return requested_status < status_count ? isc[requested_status] : NULL;
    }
    if (status < 300) {
        const int status_count = sizeof(ssc) / sizeof(ssc[0]);
        const unsigned int requested_status = status - 200;
        return requested_status < status_count ? ssc[requested_status] : NULL;
    }
    if (status < 400) {
        const int status_count = sizeof(rsc) / sizeof(rsc[0]);
        const unsigned int requested_status = status - 300;
        return requested_status < status_count ? rsc[requested_status] : NULL;
    }
    if (status < 500) {
        const int status_count = sizeof(cesc) / sizeof(cesc[0]);
        const unsigned int requested_status = status - 400;
        return requested_status < status_count ? cesc[requested_status] : NULL;
    }
    if (status < 600) {
        const int status_count = sizeof(sesc) / sizeof(sesc[0]);
        const unsigned int requested_status = status - 500;
        return requested_status < status_count ? sesc[requested_status] : NULL;
    }
    return NULL;
}

const char *http_method_string(http_method_t method)
{
    static const char *strings[] = {
        [HTTP_METHOD_GET] = "GET",
        [HTTP_METHOD_POST] = "POST",
        [HTTP_METHOD_PUT] = "PUT",
        [HTTP_METHOD_DELETE] = "DELETE",
        [HTTP_METHOD_HEAD] = "HEAD",
        [HTTP_METHOD_OPTIONS] = "OPTIONS",
        [HTTP_METHOD_CONNECT] = "CONNECT",
        [HTTP_METHOD_TRACE] = "TRACE",
        [HTTP_METHOD_PATCH] = "PATCH",
        [HTTP_METHOD_COUNT] = "UNKNOWN"};

    return strings[method < HTTP_METHOD_COUNT && method >= 0 ? method : HTTP_METHOD_COUNT];
}