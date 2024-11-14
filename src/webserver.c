#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../includes/al_socket.h"

#define BACKLOG 10      // how many pending connections queue will hold
#define BUFFER_SIZE 8192 // buffer size for received data

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ip address> <port> \n", argv[0]);
    } else {
        printf("ip address: %s, port: %s\n", argv[1], argv[2]);
    }

    int status;            // status for when we call some socket functions
    int sockfd;            // socket file descriptor
    struct addrinfo hints; // provide hints what the caller wants
    struct addrinfo *servinfo;

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t address_size;
    int new_fd; // new file descriptor for the accepted socket

    char received_buffer[15];

    // --- taken from beej's guide
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // can be both ipv4 and ipv6
    hints.ai_socktype = SOCK_STREAM; // means we use tcp
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    // ---

    parse_ip_address(argv[1], argv[2], hints, &servinfo);

    create_socket(&sockfd, servinfo);

    bind_socket(sockfd, servinfo);

    allow_socket_reuse(sockfd);

    listen_socket(sockfd, BACKLOG);

    // Main server loop
    while(1) {
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

        shutdown(new_fd, SHUT_RDWR);
    }

    // free
    freeaddrinfo(servinfo);
}
