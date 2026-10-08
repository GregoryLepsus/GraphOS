// ============================================================================
// Security & Permissions
// User/group management and access control
// ============================================================================

#include "../include/security.h"
#include "../include/heap.h"

static user_t* users = NULL;
static int user_count = 0;

// ============================================================================
// Initialization
// ============================================================================

void security_init(void) {
    // Create root user
    security_create_user("root", UID_ROOT, GID_ROOT);
}

// ============================================================================
// User Management
// ============================================================================

int security_create_user(const char* username, uint32_t uid, uint32_t gid) {
    user_t* user = (user_t*)kmalloc(sizeof(user_t));
    if (!user) return -1;

    user->uid = uid;
    user->gid = gid;
    user->capabilities = 0;

    // Copy username
    int i;
    for (i = 0; i < 31 && username[i]; i++) {
        user->username[i] = username[i];
    }
    user->username[i] = '\0';

    user_count++;
    return 0;
}

user_t* security_get_user(uint32_t uid) {
    // TODO: Implement user lookup
    return NULL;
}

// ============================================================================
// Permission Checking
// ============================================================================

int security_check_permission(user_t* user, file_perms_t* perms, uint8_t access) {
    if (!user || !perms) return 0;

    // Root can do anything
    if (user->uid == UID_ROOT) return 1;

    uint8_t effective_perms;

    // Check owner permissions
    if (user->uid == perms->owner_uid) {
        effective_perms = perms->owner_perms;
    }
    // Check group permissions
    else if (user->gid == perms->owner_gid) {
        effective_perms = perms->group_perms;
    }
    // Check other permissions
    else {
        effective_perms = perms->other_perms;
    }

    return (effective_perms & access) == access;
}
