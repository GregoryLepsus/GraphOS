// ============================================================================
// VFS Cache Layer
// File and directory caching for performance
// ============================================================================

#include "../include/vfs_cache.h"
#include "../include/heap.h"

#define CACHE_SIZE 64

static vfs_cache_entry_t* cache_entries = NULL;
static int cache_count = 0;

// ============================================================================
// Cache Initialization
// ============================================================================

void vfs_cache_init(void) {
    cache_entries = (vfs_cache_entry_t*)kmalloc(CACHE_SIZE * sizeof(vfs_cache_entry_t));
    cache_count = 0;

    for (int i = 0; i < CACHE_SIZE; i++) {
        cache_entries[i].valid = 0;
    }
}

// ============================================================================
// Cache Operations
// ============================================================================

int vfs_cache_add(const char* path, vfs_node_t* node) {
    if (!path || !node) return -1;

    // Find free slot
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache_entries[i].valid) {
            // Copy path
            int j;
            for (j = 0; j < 255 && path[j]; j++) {
                cache_entries[i].path[j] = path[j];
            }
            cache_entries[i].path[j] = '\0';

            cache_entries[i].node = node;
            cache_entries[i].access_count = 1;
            cache_entries[i].valid = 1;
            cache_count++;
            return 0;
        }
    }

    // Cache full - evict LRU
    int lru_idx = 0;
    uint32_t min_access = cache_entries[0].access_count;

    for (int i = 1; i < CACHE_SIZE; i++) {
        if (cache_entries[i].access_count < min_access) {
            min_access = cache_entries[i].access_count;
            lru_idx = i;
        }
    }

    // Replace LRU entry
    int j;
    for (j = 0; j < 255 && path[j]; j++) {
        cache_entries[lru_idx].path[j] = path[j];
    }
    cache_entries[lru_idx].path[j] = '\0';
    cache_entries[lru_idx].node = node;
    cache_entries[lru_idx].access_count = 1;

    return 0;
}

vfs_node_t* vfs_cache_lookup(const char* path) {
    if (!path) return NULL;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache_entries[i].valid) {
            // Compare paths
            int match = 1;
            for (int j = 0; j < 256; j++) {
                if (cache_entries[i].path[j] != path[j]) {
                    match = 0;
                    break;
                }
                if (path[j] == '\0') break;
            }

            if (match) {
                cache_entries[i].access_count++;
                return cache_entries[i].node;
            }
        }
    }

    return NULL;
}

void vfs_cache_invalidate(const char* path) {
    if (!path) return;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache_entries[i].valid) {
            int match = 1;
            for (int j = 0; j < 256; j++) {
                if (cache_entries[i].path[j] != path[j]) {
                    match = 0;
                    break;
                }
                if (path[j] == '\0') break;
            }

            if (match) {
                cache_entries[i].valid = 0;
                cache_count--;
                return;
            }
        }
    }
}
