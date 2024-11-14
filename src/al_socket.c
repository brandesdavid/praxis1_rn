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

extern const char* get_status_text(enum HTTP_STATUS_CODE code) {
    switch (code) {
        case HTTP_OK:
            return "200 OK";
        case HTTP_BAD_REQUEST:
            return "400 Bad Request";
        case HTTP_NOT_FOUND:
            return "404 Not Found";
        case HTTP_INTERNAL_ERROR:
            return "500 Internal Server Error";
        default:
            return "500 Internal Server Error";
    }
}

extern enum AL_SOCKET_FAULT send_http_response(int client_fd, struct http_response *response) {
    char response_buffer[MAX_BODY_LENGTH + 1024];  // Extra space for headers
    int written = snprintf(response_buffer, sizeof(response_buffer),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        get_status_text(response->status_code),
        response->content_type,
        response->body_length,
        response->body
    );

    if (send(client_fd, response_buffer, written, 0) == -1) {
        return AL_SOCKET_SEND_ERROR;
    }
    
    return AL_SOCKET_FAULT_NONE;
}

extern enum AL_SOCKET_FAULT handle_client_connection(int client_fd) {
    char received_buffer[MAX_BODY_LENGTH];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_fd, received_buffer, 
           sizeof(received_buffer) - 1, 0)) > 0) {
        
        received_buffer[bytes_received] = '\0';

        struct http_response response = {
            .status_code = HTTP_OK,
            .body_length = 0
        };
        
        // how to set different responses based on conditions
        if (strstr(received_buffer, "GET /") != NULL) {
            strcpy(response.content_type, "text/html");
            strcpy(response.body, "Hello, World!");
            response.body_length = strlen(response.body);
        }
        
        // send back to se client
        send_http_response(client_fd, &response);
    }

    return AL_SOCKET_FAULT_NONE;
}
