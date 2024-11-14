#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../includes/al_socket.h"

#define BACKLOG 10      // how many pending connections queue will hold
#define BUFFER_SIZE 100 // buffer size for received data
int status;             // status for when we call some socket functions
int sockfd;             // socket file descriptor
struct addrinfo hints;  // provide hints what the caller wants
struct addrinfo *servinfo;

struct sockaddr_storage their_addr; // connector's address information
socklen_t address_size;
int new_fd; // new file descriptor for the accepted socket

char received_buffer[15];

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <ip address> <port> \n", argv[0]);
    } else {
        printf("ip address: %s, port: %s\n", argv[1], argv[2]);
    }

    // --- taken from beej's guide
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // can be both ipv4 and ipv6
    hints.ai_socktype = SOCK_STREAM; // means we use tcp
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    // ---

    // parse the ip address and port from the args
    parse_ip_address(argv[1], argv[2], hints, &servinfo);

    // create a socket
    create_socket(&sockfd, servinfo);

    // bind the socket

    bind_socket(sockfd, servinfo);

    // --- taken from beej's guide -- to avoid "address already in use" error

    allow_socket_reuse(sockfd);
    // ---

    // listen
    listen_socket(sockfd, BACKLOG);

    // accept incoming connections

    while (1) {
        address_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &address_size);

        if ((status = recv(new_fd, received_buffer, 5, 0)) == -1) {
            perror("recv failed");
            exit(1);
        }

        if (status == 0)
            printf("client closed the connection\n");

        send(new_fd, "Hello, world!", 13, 0);

        printf("received: %s\n", received_buffer);
    };

    // free
    freeaddrinfo(servinfo);
}
