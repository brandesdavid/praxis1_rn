#ifndef AL_SOCKET_H
#define AL_SOCKET_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

enum AL_SOCKET_FAULT {
    AL_SOCKET_FAULT_NONE = 0,
    AL_SOCKET_GETADDRINFO_ERROR,
    AL_SOCKET_CREATE_SOCKET_ERROR,
    AL_SOCKET_BIND_ERROR,
    AL_SOCKET_LISTEN_ERROR,
    AL_SOCKET_ACCEPT_ERROR,
    AL_SOCKET_RECV_ERROR,
    AL_SOCKET_SEND_ERROR,
    AL_SOCKET_SETSOCKOPT_ERROR
};

#define MAX_HEADERS 100
#define MAX_HEADER_LENGTH 1024
#define MAX_BODY_LENGTH 8192

enum HTTP_STATUS_CODE {
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_NO_CONTENT = 204,
    HTTP_BAD_REQUEST = 400,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_INTERNAL_ERROR = 500
};

struct http_response {
    enum HTTP_STATUS_CODE status_code;
    char content_type[128];
    char body[MAX_BODY_LENGTH];
    size_t body_length;
};

enum AL_SOCKET_FAULT parse_ip_address(char *ip_address, char *port,
                                    struct addrinfo hints,
                                    struct addrinfo **servinfo);
enum AL_SOCKET_FAULT create_socket(int *sockfd, struct addrinfo *servinfo);
enum AL_SOCKET_FAULT bind_socket(int sockfd, struct addrinfo *servinfo);
enum AL_SOCKET_FAULT allow_socket_reuse(int sockfd);
enum AL_SOCKET_FAULT listen_socket(int sockfd, int amount_of_connections);
enum AL_SOCKET_FAULT handle_client_connection(int client_fd);


#endif // AL_SOCKET_H
