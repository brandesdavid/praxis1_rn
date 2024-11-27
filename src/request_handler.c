#include "../includes/request_handler.h"
#include "../includes/http_utils.h"
#include "../includes/resource_manager.h"
#include <string.h>
#include <stdlib.h>

enum AL_SOCKET_FAULT handle_http_request(const char *request,
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

    // check if we have a valid method (in the test head e.g)
    if (strcmp(method, "GET") != 0 && 
        strcmp(method, "PUT") != 0 && 
        strcmp(method, "DELETE") != 0) {
        prepare_response(&response, HTTP_NOT_IMPLEMENTED);
        return send_http_response(client_fd, &response);
    }

    // check if we have a content length header, and if so, parse it
    const char *content_length_header = strstr(request, "Content-Length: ");
    if (content_length_header) {
        sscanf(content_length_header, "Content-Length: %15s",
               content_length_str);
        content_length = atoi(content_length_str);
    }

    // find the start of the body
    body_start = strstr(request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
    }

    if (strncmp(uri, "/dynamic/", 9) == 0) {
        return handle_dynamic_request(method, uri, body_start, content_length, client_fd);
    }

    if (strncmp(uri, "/static/", 8) == 0) {
        // only allow GET for static content
        if (strcmp(method, "GET") != 0) {
            prepare_response(&response, HTTP_NOT_IMPLEMENTED);
            return send_http_response(client_fd, &response);
        }
        return handle_static_request(method, uri, client_fd);
    }

    prepare_response(&response, HTTP_NOT_FOUND);
    return send_http_response(client_fd, &response);
}

enum HTTP_STATUS_CODE
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

enum AL_SOCKET_FAULT handle_static_request(const char *method, const char *uri,
                                                  int client_fd) {
    struct http_response response;

    if (strcmp(method, "GET") == 0) {
        enum HTTP_STATUS_CODE status = handle_static_content(uri, &response);
        response.status_code = status;
        return send_http_response(client_fd, &response);
    }

    prepare_response(&response, HTTP_NOT_IMPLEMENTED);
    return send_http_response(client_fd, &response);
}

enum AL_SOCKET_FAULT handle_put_request(struct dynamic_resource *resource, const char *uri,
                                               const char *body_start, int content_length,
                                               int client_fd) {
    struct http_response response;

    if (!body_start || content_length == 0) {
        prepare_response(&response, HTTP_BAD_REQUEST);
        return send_http_response(client_fd, &response);
    }

    if (resource) {
        // update the resoucre !!!
        memcpy(resource->content, body_start, content_length);
        resource->content_length = content_length;
        prepare_response(&response, HTTP_NO_CONTENT);
    } else {
        // create a new resource !!!
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
    return send_http_response(client_fd, &response);
}

enum AL_SOCKET_FAULT handle_delete_request(struct dynamic_resource *resource,
                                                  const char *uri, int client_fd) {
    struct http_response response;

    if (resource) {
        delete_resource(uri);
        prepare_response(&response, HTTP_NO_CONTENT);
    } else {
        prepare_response(&response, HTTP_NOT_FOUND);
    }
    return send_http_response(client_fd, &response);
}
enum AL_SOCKET_FAULT handle_dynamic_request(const char *method, const char *uri,
                                                   const char *body_start, int content_length,
                                                   int client_fd) {
    struct http_response response;
    struct dynamic_resource *resource = find_resource(uri);

    if (strcmp(method, "GET") == 0) {
        if (resource) {
            prepare_response(&response, HTTP_OK);
            memcpy(response.body, resource->content, resource->content_length);
            response.body_length = resource->content_length;
        } else {
            prepare_response(&response, HTTP_NOT_FOUND);
        }
        return send_http_response(client_fd, &response);
    }

    if (strcmp(method, "PUT") == 0) {
        return handle_put_request(resource, uri, body_start, content_length, client_fd);
    }

    if (strcmp(method, "DELETE") == 0) {
        return handle_delete_request(resource, uri, client_fd);
    }

    prepare_response(&response, HTTP_NOT_IMPLEMENTED);
    return send_http_response(client_fd, &response);
}