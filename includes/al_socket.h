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

extern enum AL_SOCKET_FAULT parse_ip_address(char *ip_address, char *port,
                                             struct addrinfo hints,
                                             struct addrinfo **servinfo);

extern enum AL_SOCKET_FAULT create_socket(int *sockfd,
                                          struct addrinfo *servinfo);

extern enum AL_SOCKET_FAULT bind_socket(int sockfd, struct addrinfo *servinfo);

extern enum AL_SOCKET_FAULT allow_socket_reuse(int sockfd);

extern enum AL_SOCKET_FAULT listen_socket(int sockfd,
                                          int amount_of_connections);

#endif // AL_SOCKET_H
