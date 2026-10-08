// ============================================================================
// VFS Links - Symbolic and Hard Links
// Support for file system links
// ============================================================================

#include "../include/vfs_links.h"
#include "../include/heap.h"
#include "../include/vfs.h"

static symlink_t* symlinks = NULL;
static hardlink_t* hardlinks = NULL;

// ============================================================================
// Symbolic Links
// ============================================================================

int vfs_symlink_create(const char* target, const char* linkpath) {
    if (!target || !linkpath) return -1;

    symlink_t* link = (symlink_t*)kmalloc(sizeof(symlink_t));
    if (!link) return -1;

    // Copy target path
    int i;
    for (i = 0; i < 255 && target[i]; i++) {
        link->target[i] = target[i];
    }
    link->target[i] = '\0';

    // Copy link path
    for (i = 0; i < 255 && linkpath[i]; i++) {
        link->linkpath[i] = linkpath[i];
    }
    link->linkpath[i] = '\0';

    link->next = symlinks;
    symlinks = link;

    return 0;
}

const char* vfs_symlink_resolve(const char* path) {
    if (!path) return NULL;

    symlink_t* link = symlinks;
    while (link) {
        int match = 1;
        for (int i = 0; i < 256; i++) {
            if (link->linkpath[i] != path[i]) {
                match = 0;
                break;
            }
            if (path[i] == '\0') break;
        }

        if (match) {
            return link->target;
        }

        link = link->next;
    }

    return path;  // Not a symlink, return original
}

int vfs_symlink_remove(const char* linkpath) {
    if (!linkpath) return -1;

    symlink_t* prev = NULL;
    symlink_t* curr = symlinks;

    while (curr) {
        int match = 1;
        for (int i = 0; i < 256; i++) {
            if (curr->linkpath[i] != linkpath[i]) {
                match = 0;
                break;
            }
            if (linkpath[i] == '\0') break;
        }

        if (match) {
            if (prev) {
                prev->next = curr->next;
            } else {
                symlinks = curr->next;
            }
            kfree(curr);
            return 0;
        }

        prev = curr;
        curr = curr->next;
    }

    return -1;
}

// ============================================================================
// Hard Links
// ============================================================================

int vfs_hardlink_create(vfs_node_t* target, const char* linkpath) {
    if (!target || !linkpath) return -1;

    hardlink_t* link = (hardlink_t*)kmalloc(sizeof(hardlink_t));
    if (!link) return -1;

    link->target = target;

    // Copy link path
    int i;
    for (i = 0; i < 255 && linkpath[i]; i++) {
        link->linkpath[i] = linkpath[i];
    }
    link->linkpath[i] = '\0';

    link->refcount = 1;
    link->next = hardlinks;

    hardlinks = link;

    return 0;
}

vfs_node_t* vfs_hardlink_resolve(const char* path) {
    if (!path) return NULL;

    hardlink_t* link = hardlinks;
    while (link) {
        int match = 1;
        for (int i = 0; i < 256; i++) {
            if (link->linkpath[i] != path[i]) {
                match = 0;
                break;
            }
            if (path[i] == '\0') break;
        }

        if (match) {
            return link->target;
        }

        link = link->next;
    }

    return NULL;
}

int vfs_hardlink_remove(const char* linkpath) {
    if (!linkpath) return -1;

    hardlink_t* prev = NULL;
    hardlink_t* curr = hardlinks;

    while (curr) {
        int match = 1;
        for (int i = 0; i < 256; i++) {
            if (curr->linkpath[i] != linkpath[i]) {
                match = 0;
                break;
            }
            if (linkpath[i] == '\0') break;
        }

        if (match) {
            curr->refcount--;

            if (curr->refcount == 0) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    hardlinks = curr->next;
                }
                kfree(curr);
            }

            return 0;
        }

        prev = curr;
        curr = curr->next;
    }

    return -1;
}
