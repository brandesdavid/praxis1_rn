#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "al_socket.h"

enum AL_SOCKET_FAULT handle_http_request(const char *request, int client_fd);
enum AL_SOCKET_FAULT handle_static_request(const char *method, const char *uri, int client_fd);
enum AL_SOCKET_FAULT handle_dynamic_request(const char *method, const char *uri, 
                                          const char *body_start, int content_length, 
                                          int client_fd);

#endif 