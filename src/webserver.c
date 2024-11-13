#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BACKLOG 10     // how many pending connections queue will hold
int status;            // status for when we call some socket functions
int sockfd;            // socket file descriptor
struct addrinfo hints; // provide hints what the caller wants
struct addrinfo *servinfo;

struct sockaddr_storage their_addr; // connector's address information
socklen_t address_size;
int new_fd; // new file descriptor for the accepted socket

char *received_buffer[100];

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
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    // create a socket
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
                    servinfo->ai_protocol);

    // bind the socket
    if ((status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != 0)) {
        fprintf(stderr, "bind: %s\n", gai_strerror(status));
        return 1;
    }

    // --- taken from beej's guide -- to avoid "address already in use" error
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    }
    // ---

    // listen
    if ((status = listen(sockfd, BACKLOG)) != 0) {
        fprintf(stderr, "listen: %s\n", gai_strerror(status));
        return 1;
    }

    // accept incoming connections
    address_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &address_size);

    if ((status = recv(new_fd, received_buffer, sizeof received_buffer, 0)) ==
        -1) {
        perror("recv");
        exit(1);
    }

    if (status == 0)
        printf("client closed the connection\n");

    send(new_fd, "Hello, world!", 13, 0);

    // free
    freeaddrinfo(servinfo);
}
