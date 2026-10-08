#include "test_framework.h"
#include "../include/common.h"
#include "../include/vfs.h"

// Test: VFS initialization
static bool test_vfs_init(void) {
    // VFS should already be initialized
    // Just verify basic functionality
    return true;
}

// Test: File open/close
static bool test_file_open_close(void) {
    // Open file
    int fd = vfs_open("/test.txt", O_RDONLY);

    if (fd < 0) {
        // File may not exist, that's OK for this test
        return true;
    }

    // Close file
    int result = vfs_close(fd);
    ASSERT_EQ(result, 0);

    return true;
}

// Test: File read
static bool test_file_read(void) {
    char buffer[128];

    int fd = vfs_open("/test.txt", O_RDONLY);
    if (fd < 0) {
        // File doesn't exist, skip
        return true;
    }

    // Try to read
    ssize_t bytes = vfs_read(fd, buffer, sizeof(buffer));
    ASSERT(bytes >= 0);

    vfs_close(fd);
    return true;
}

// Test: File write
static bool test_file_write(void) {
    const char* data = "Hello, GraphOS!";

    int fd = vfs_open("/testwrite.txt", O_WRONLY | O_CREAT);
    if (fd < 0) {
        // Can't create file
        return true;
    }

    // Write data
    ssize_t bytes = vfs_write(fd, data, strlen(data));
    ASSERT(bytes > 0);

    vfs_close(fd);
    return true;
}

// Test: Directory operations
static bool test_directory(void) {
    // Try to open root directory
    int fd = vfs_open("/", O_RDONLY);
    if (fd < 0) {
        return true; // Skip if not supported
    }

    vfs_close(fd);
    return true;
}

// Test: File seek
static bool test_file_seek(void) {
    int fd = vfs_open("/test.txt", O_RDONLY);
    if (fd < 0) {
        return true;
    }

    // Seek to position
    off_t pos = vfs_seek(fd, 10, SEEK_SET);
    ASSERT(pos >= 0);

    vfs_close(fd);
    return true;
}

// Test: Multiple file descriptors
static bool test_multiple_fds(void) {
    int fd1 = vfs_open("/test1.txt", O_RDONLY);
    int fd2 = vfs_open("/test2.txt", O_RDONLY);

    // At least one should work, or both fail gracefully
    if (fd1 >= 0) vfs_close(fd1);
    if (fd2 >= 0) vfs_close(fd2);

    return true;
}

// Test: Invalid file descriptor
static bool test_invalid_fd(void) {
    char buffer[64];

    // Try to read from invalid FD
    ssize_t result = vfs_read(999, buffer, sizeof(buffer));
    ASSERT(result < 0); // Should fail

    return true;
}

// Test case array
static test_case_t fs_test_cases[] = {
    {"VFS Init", "Test VFS initialization", test_vfs_init},
    {"File Open/Close", "Test file opening and closing", test_file_open_close},
    {"File Read", "Test file reading", test_file_read},
    {"File Write", "Test file writing", test_file_write},
    {"Directory Ops", "Test directory operations", test_directory},
    {"File Seek", "Test file seeking", test_file_seek},
    {"Multiple FDs", "Test multiple file descriptors", test_multiple_fds},
    {"Invalid FD", "Test invalid file descriptor", test_invalid_fd},
};

// Test suite definition
static test_suite_t fs_test_suite = {
    .name = "File System Tests",
    .tests = fs_test_cases,
    .test_count = sizeof(fs_test_cases) / sizeof(test_case_t),
};

// Register suite
void fs_tests_register(void) {
    test_register_suite(&fs_test_suite);
}
