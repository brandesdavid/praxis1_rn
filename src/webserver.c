#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../includes/al_socket.h"

#define BACKLOG 10
#define BUFFER_SIZE 8192

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ip address> <port> \n", argv[0]);
        return 1;
    }

    int sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct sockaddr_storage their_addr;
    socklen_t address_size;
    int new_fd;

    // Initialize hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (parse_ip_address(argv[1], argv[2], hints, &servinfo) != AL_SOCKET_FAULT_NONE) {
        return 1;
    }

    if (create_socket(&sockfd, servinfo) != AL_SOCKET_FAULT_NONE) {
        freeaddrinfo(servinfo);
        return 1;
    }

    if (allow_socket_reuse(sockfd) != AL_SOCKET_FAULT_NONE) {
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }

    if (bind_socket(sockfd, servinfo) != AL_SOCKET_FAULT_NONE) {
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }

    if (listen_socket(sockfd, BACKLOG) != AL_SOCKET_FAULT_NONE) {
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }

    freeaddrinfo(servinfo);

    while (1) {
        // create a new socket for client, and accept the connection
        address_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &address_size);
        if (new_fd == -1) {
            perror("accept failed");
            continue;
        }

        // handle the client connection with abstraction layer of socket

        enum AL_SOCKET_FAULT result = handle_client_connection(new_fd);
        if (result != AL_SOCKET_FAULT_NONE) {
            fprintf(stderr, "Error handling client connection: %d\n", result);
        }

        close(new_fd);
    }

    close(sockfd);
    return 0;
}
