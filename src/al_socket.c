#include "../includes/al_socket.h"

extern enum AL_SOCKET_FAULT parse_ip_address(char *ip_address, char *port,
                                             struct addrinfo hints,
                                             struct addrinfo **servinfo) {
    // parse the ip address and port from the args
    int status;
    if ((status = getaddrinfo(ip_address, port, &hints, &*servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return AL_SOCKET_GETADDRINFO_ERROR;
    }
    return AL_SOCKET_FAULT_NONE;
}

extern enum AL_SOCKET_FAULT create_socket(int *sockfd,
                                          struct addrinfo *servinfo) {
    // create a socket
    if ((*sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
                          servinfo->ai_protocol)) == -1) {
        perror("socket");
        return AL_SOCKET_CREATE_SOCKET_ERROR;
    }
    return AL_SOCKET_FAULT_NONE;
}

extern enum AL_SOCKET_FAULT bind_socket(int sockfd, struct addrinfo *servinfo) {
    // bind the socket
    int status;
    if ((status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != 0)) {
        fprintf(stderr, "bind: %s\n", gai_strerror(status));
        return AL_SOCKET_BIND_ERROR;
    }
    return AL_SOCKET_FAULT_NONE;
}

extern enum AL_SOCKET_FAULT allow_socket_reuse(int sockfd) {
    // --- taken from beej's guide -- to avoid "address already in use" error
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        perror("setsockopt");
        return AL_SOCKET_SETSOCKOPT_ERROR;
    }
    return AL_SOCKET_FAULT_NONE;
}

extern enum AL_SOCKET_FAULT listen_socket(int sockfd,
                                          int amount_of_connections) {
    // listen
    int status;
    if ((status = listen(sockfd, amount_of_connections)) != 0) {
        fprintf(stderr, "listen: %s\n", gai_strerror(status));
        return AL_SOCKET_LISTEN_ERROR;
    }
    return AL_SOCKET_FAULT_NONE;
}
