#include "../includes/resource_manager.h"
#include <string.h>

#define MAX_RESOURCES 100
#define MAX_PATH_LENGTH 256
#define MAX_CONTENT_LENGTH 8192

struct dynamic_resource resources[MAX_RESOURCES] = {0};

struct dynamic_resource *find_resource(const char *path) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].in_use && strcmp(resources[i].path, path) == 0) {
            return &resources[i];
        }
    }
    return NULL;
}

struct dynamic_resource *create_resource() {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!resources[i].in_use) {
            resources[i].in_use = true;
            return &resources[i];
        }
    }
    return NULL;
}

void delete_resource(const char *path) {
    struct dynamic_resource *resource = find_resource(path);
    if (resource != NULL) {
        resource->in_use = false;
    }
}
