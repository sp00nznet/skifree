/*
 * SkiFree Resource System
 * Loads resources from disk (mod directory) with embedded fallback.
 */

#include "resources.h"
#include "embedded_resources.h"
#include <stdio.h>
#include <string.h>

static char mod_dir[512] = {0};
static int mod_dir_set = 0;

void resource_init(const char *dir) {
    if (dir && dir[0]) {
        strncpy(mod_dir, dir, sizeof(mod_dir) - 1);
        mod_dir[sizeof(mod_dir) - 1] = '\0';
        mod_dir_set = 1;
        printf("[resources] Mod directory: %s\n", mod_dir);
    } else {
        mod_dir[0] = '\0';
        mod_dir_set = 0;
        printf("[resources] Using embedded resources only\n");
    }
}

void resource_shutdown(void) {
    mod_dir[0] = '\0';
    mod_dir_set = 0;
}

const char *resource_get_mod_dir(void) {
    return mod_dir_set ? mod_dir : NULL;
}

static Resource *load_from_disk(const char *name) {
    FILE *f;
    char path[1024];
    long file_size;
    char *buf;
    Resource *res;

    if (!mod_dir_set) return NULL;

    snprintf(path, sizeof(path), "%s/%s", mod_dir, name);

    f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(f);
        return NULL;
    }

    buf = (char *)malloc(file_size);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, file_size, f) != (size_t)file_size) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);

    res = (Resource *)malloc(sizeof(Resource));
    if (!res) {
        free(buf);
        return NULL;
    }

    res->name = name;
    res->len = (int)file_size;
    res->content = buf;
    res->from_disk = 1;

    printf("[resources] Loaded from disk: %s (%d bytes)\n", path, res->len);
    return res;
}

static Resource *load_from_embedded(const char *name) {
    embedded_resource_t *embedded;
    Resource *res;

    embedded = get_embedded_resource_by_name((char *)name);
    if (!embedded) return NULL;

    res = (Resource *)malloc(sizeof(Resource));
    if (!res) return NULL;

    res->name = embedded->name;
    res->len = embedded->len;
    res->content = embedded->content;
    res->from_disk = 0;

    return res;
}

Resource *resource_load(const char *name) {
    Resource *res;

    /* Try disk first (mod override) */
    res = load_from_disk(name);
    if (res) return res;

    /* Fall back to embedded */
    res = load_from_embedded(name);
    return res;
}

void resource_free(Resource *res) {
    if (!res) return;
    if (res->from_disk && res->content) {
        free((void *)res->content);
    }
    free(res);
}
