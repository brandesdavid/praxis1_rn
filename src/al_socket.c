#include "../includes/al_socket.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char *get_status_text(enum HTTP_STATUS_CODE code) {
    switch (code) {
    case HTTP_OK:
        return "200 OK";
    case HTTP_CREATED:
        return "201 Created";
    case HTTP_NO_CONTENT:
        return "204 No Content";
    case HTTP_BAD_REQUEST:
        return "400 Bad Request";
    case HTTP_FORBIDDEN:
        return "403 Forbidden";
    case HTTP_NOT_FOUND:
        return "404 Not Found";
    case HTTP_NOT_IMPLEMENTED:
        return "501 Not Implemented";
    case HTTP_INTERNAL_ERROR:
        return "500 Internal Server Error";
    default:
        return "500 Internal Server Error";
    }
}

#define MAX_RESOURCES 100
#define MAX_PATH_LENGTH 256
#define MAX_CONTENT_LENGTH 8192

struct dynamic_resource {
    char path[MAX_PATH_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    size_t content_length;
    bool in_use;
};

static struct dynamic_resource resources[MAX_RESOURCES] = {0};

static struct dynamic_resource *find_resource(const char *path) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].in_use && strcmp(resources[i].path, path) == 0) {
            return &resources[i];
        }
    }
    return NULL;
}

static struct dynamic_resource *create_resource() {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!resources[i].in_use) {
            resources[i].in_use = true;
            return &resources[i];
        }
    }
    return NULL;
}

static void delete_resource(const char *path) {
    struct dynamic_resource *resource = find_resource(path);
    if (resource != NULL) {
        resource->in_use = false;
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

extern enum AL_SOCKET_FAULT send_http_response(int client_fd,
                                               struct http_response *response) {
    char response_buffer[MAX_BODY_LENGTH + 1024]; // Extra space for headers
    int written =
        snprintf(response_buffer, sizeof(response_buffer),
                 "HTTP/1.1 %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 get_status_text(response->status_code), response->content_type,
                 response->body_length, response->body);

    if (send(client_fd, response_buffer, written, 0) == -1) {
        return AL_SOCKET_SEND_ERROR;
    }

    return AL_SOCKET_FAULT_NONE;
}

static enum HTTP_STATUS_CODE
handle_static_content(const char *uri, struct http_response *response) {
    if (strcmp(uri, "/static/foo") == 0) {
        strcpy(response->body, "Foo");
        response->body_length = 3;
        return HTTP_OK;
    }
    if (strcmp(uri, "/static/bar") == 0) {
        strcpy(response->body, "Bar");
        response->body_length = 3;
        return HTTP_OK;
    }
    if (strcmp(uri, "/static/baz") == 0) {
        strcpy(response->body, "Baz");
        response->body_length = 3;
        return HTTP_OK;
    }
    return HTTP_NOT_FOUND;
}

static void prepare_response(struct http_response *response,
                             enum HTTP_STATUS_CODE status) {
    memset(response, 0, sizeof(*response));
    strcpy(response->content_type, "text/plain");
    response->status_code = status;
}

static enum AL_SOCKET_FAULT handle_http_request(const char *request,
                                                int client_fd) {
    char method[8] = {0};
    char uri[256] = {0};
    char http_version[16] = {0};
    char content_length_str[16] = {0};
    int content_length = 0;
    char *body_start;

    struct http_response response;
    prepare_response(&response, HTTP_BAD_REQUEST);

    // Parse request line
    if (sscanf(request, "%7s %255s %15s", method, uri, http_version) != 3) {
        return send_http_response(client_fd, &response);
    }

    // Handle HEAD method first
    if (strcmp(method, "HEAD") == 0) {
        prepare_response(&response, HTTP_NOT_IMPLEMENTED);
        return send_http_response(client_fd, &response);
    }

    // Find Content-Length header if present
    const char *content_length_header = strstr(request, "Content-Length: ");
    if (content_length_header) {
        sscanf(content_length_header, "Content-Length: %15s",
               content_length_str);
        content_length = atoi(content_length_str);
    }

    // Find start of body
    body_start = strstr(request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
    }

    if (strncmp(uri, "/dynamic/", 9) == 0) {
        struct dynamic_resource *resource = find_resource(uri);

        if (strcmp(method, "GET") == 0) {
            if (resource) {
                prepare_response(&response, HTTP_OK);
                memcpy(response.body, resource->content,
                       resource->content_length);
                response.body_length = resource->content_length;
            } else {
                prepare_response(&response, HTTP_NOT_FOUND);
            }
        } else if (strcmp(method, "PUT") == 0) {
            if (!body_start || content_length == 0) {
                prepare_response(&response, HTTP_BAD_REQUEST);
            } else {
                if (resource) {
                    // Update existing resource
                    memcpy(resource->content, body_start, content_length);
                    resource->content_length = content_length;
                    prepare_response(&response, HTTP_NO_CONTENT);
                } else {
                    // Create new resource
                    resource = create_resource();
                    if (resource) {
                        strncpy(resource->path, uri, MAX_PATH_LENGTH - 1);
                        memcpy(resource->content, body_start, content_length);
                        resource->content_length = content_length;
                        prepare_response(&response, HTTP_CREATED);
                    } else {
                        prepare_response(&response, HTTP_INTERNAL_ERROR);
                    }
                }
            }
        } else if (strcmp(method, "DELETE") == 0) {
            if (resource) {
                delete_resource(uri);
                prepare_response(&response, HTTP_NO_CONTENT);
            } else {
                prepare_response(&response, HTTP_NOT_FOUND);
            }
        } else {
            prepare_response(&response, HTTP_NOT_IMPLEMENTED);
        }
    } else if (strncmp(uri, "/static/", 8) == 0) {
        // Existing static content handling
        if (strcmp(method, "GET") == 0) {
            enum HTTP_STATUS_CODE status =
                handle_static_content(uri, &response);
            response.status_code = status;
        } else {
            prepare_response(&response, HTTP_NOT_IMPLEMENTED);
        }
    } else {
        prepare_response(&response, HTTP_NOT_FOUND);
    }

    return send_http_response(client_fd, &response);
}

extern enum AL_SOCKET_FAULT handle_client_connection(int client_fd) {
    char received_buffer[MAX_BODY_LENGTH];
    char request[MAX_BODY_LENGTH] = {0};
    size_t total_received = 0;
    ssize_t bytes_received;

    while ((bytes_received =
                recv(client_fd, received_buffer,
                     sizeof(received_buffer) - 1 - total_received, 0)) > 0) {

        strncat(request + total_received, received_buffer, bytes_received);
        total_received += bytes_received;
        request[total_received] = '\0';

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
