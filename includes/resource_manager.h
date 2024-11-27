#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stdbool.h>
#include <stdlib.h>

#define MAX_RESOURCES 100
#define MAX_PATH_LENGTH 256
#define MAX_CONTENT_LENGTH 8192

struct dynamic_resource {
    char path[MAX_PATH_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    size_t content_length;
    bool in_use;
};

struct dynamic_resource *find_resource(const char *path);
struct dynamic_resource *create_resource(void);
void delete_resource(const char *path);

#endif 