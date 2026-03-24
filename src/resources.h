/*
 * SkiFree Resource System
 * Loads resources from disk (mod directory) with embedded fallback.
 */

#ifndef SKIFREE_RESOURCES_H
#define SKIFREE_RESOURCES_H

#include <stdlib.h>

typedef struct {
    const char *name;
    int len;
    const char *content;
    int from_disk; /* 1 if malloc'd and needs freeing */
} Resource;

/* Initialize the resource system. mod_dir may be NULL for embedded-only. */
void resource_init(const char *mod_dir);

/* Shut down and free cached resources. */
void resource_shutdown(void);

/* Load a resource by name. Checks disk first, falls back to embedded.
 * Returns NULL if not found in either location.
 * Caller must call resource_free() when done. */
Resource *resource_load(const char *name);

/* Free a resource returned by resource_load(). */
void resource_free(Resource *res);

/* Get the current mod directory, or NULL if none set. */
const char *resource_get_mod_dir(void);

#endif /* SKIFREE_RESOURCES_H */
