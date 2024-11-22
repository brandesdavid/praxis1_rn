#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../includes/al_socket.h"

#define BACKLOG 10       // how many pending connections queue will hold
#define BUFFER_SIZE 8192 // buffer size for received data

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ip address> <port> \n", argv[0]);
        return 1;
    }

    int sockfd;            // socket file descriptor
    struct addrinfo hints; // provide hints what the caller wants
    struct addrinfo *servinfo;
    struct sockaddr_storage their_addr;
    socklen_t address_size;
    int new_fd;

    // Initialize hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // Fill in IP

    // Set up socket before binding
    if (parse_ip_address(argv[1], argv[2], hints, &servinfo) != AL_SOCKET_FAULT_NONE) {
        return 1;
    }

    // Allow socket reuse before creating the socket
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

    freeaddrinfo(servinfo); // Free this early as we don't need it anymore

    // Main server loop
    while (1) {
        address_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &address_size);
        if (new_fd == -1) {
            perror("accept failed");
            continue;
        }

        enum AL_SOCKET_FAULT result = handle_client_connection(new_fd);
        if (result != AL_SOCKET_FAULT_NONE) {
            fprintf(stderr, "Error handling client connection: %d\n", result);
        }

        close(new_fd);
    }

    close(sockfd);
    return 0;
}
