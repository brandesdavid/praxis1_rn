#include "../includes/http_utils.h"

extern const char *get_status_text(enum HTTP_STATUS_CODE code) {
    switch (code) {
    case HTTP_OK:
        return "200 OK";
    case HTTP_CREATED:
        return "201 Created";
    case HTTP_NO_CONTENT:
        return "204 No Content";
    case HTTP_BAD_REQUEST:
        return "400 Bad Request";
    case HTTP_FORBIDDEN:
        return "403 Forbidden";
    case HTTP_NOT_FOUND:
        return "404 Not Found";
    case HTTP_NOT_IMPLEMENTED:
        return "501 Not Implemented";
    case HTTP_INTERNAL_ERROR:
        return "500 Internal Server Error";
    default:
        return "500 Internal Server Error";
    }
}

extern enum AL_SOCKET_FAULT send_http_response(int client_fd,
                                               struct http_response *response) {
    char response_buffer[MAX_BODY_LENGTH + 1024]; // extra space for headers
    int written =
        snprintf(response_buffer, sizeof(response_buffer),
                 "HTTP/1.1 %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 get_status_text(response->status_code), response->content_type,
                 response->body_length, response->body);

    if (send(client_fd, response_buffer, written, 0) == -1) {
        return AL_SOCKET_SEND_ERROR;
    }

    return AL_SOCKET_FAULT_NONE;
}

void prepare_response(struct http_response *response,
                             enum HTTP_STATUS_CODE status) {
    memset(response, 0, sizeof(*response));
    strcpy(response->content_type, "text/plain");
    response->status_code = status;
}
