#include "../includes/al_socket.h"
#include "../includes/request_handler.h"

extern enum AL_SOCKET_FAULT handle_client_connection(int client_fd) {
    char received_buffer[MAX_BODY_LENGTH];
    char request[MAX_BODY_LENGTH] = {0};
    size_t total_received = 0;
    ssize_t bytes_received;

    // this weird equation is to receive the request from the client in a loop
    while ((bytes_received =
                recv(client_fd, received_buffer,
                     sizeof(received_buffer) - 1 - total_received, 0)) > 0) {

        strncat(request + total_received, received_buffer, bytes_received);
        total_received += bytes_received;
        request[total_received] = '\0';

        // if the request is complete (meaning \r\n\r\n is there), handle it
        if (strstr(request, "\r\n\r\n")) {
            enum AL_SOCKET_FAULT result =
                handle_http_request(request, client_fd);

            if (result != AL_SOCKET_FAULT_NONE) {
                return result;
            }

            // clear buffers for next request
            memset(request, 0, sizeof(request));
            total_received = 0;
        }
    }

    if (bytes_received == -1) {
        return AL_SOCKET_RECV_ERROR;
    } else {
        return AL_SOCKET_FAULT_NONE;
    }
}

extern enum AL_SOCKET_FAULT parse_ip_address(char *ip_address, char *port,
                                             struct addrinfo hints,
                                             struct addrinfo **servinfo) {
    // parse the ip address and port from the args
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // Fill in IP

    if ((status = getaddrinfo(ip_address, port, &hints, servinfo)) != 0) {
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
    int yes = 1;
    // Set both SO_REUSEADDR and SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        return AL_SOCKET_SETSOCKOPT_ERROR;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1) {
        perror("setsockopt SO_REUSEPORT");
        return AL_SOCKET_SETSOCKOPT_ERROR;
    }
#endif
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

