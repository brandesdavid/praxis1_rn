#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include "al_socket.h"

extern const char *get_status_text(enum HTTP_STATUS_CODE code);
void prepare_response(struct http_response *response, enum HTTP_STATUS_CODE status);
enum AL_SOCKET_FAULT send_http_response(int client_fd, struct http_response *response);

#endif 