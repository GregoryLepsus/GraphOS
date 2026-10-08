#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>

// Test result codes
#define TEST_PASS 0
#define TEST_FAIL 1
#define TEST_SKIP 2

// Test case structure
typedef struct {
    const char* name;
    const char* description;
    bool (*test_func)(void);
    bool passed;
    bool skipped;
    uint32_t duration_ms;
} test_case_t;

// Test suite structure
typedef struct {
    const char* name;
    test_case_t* tests;
    uint32_t test_count;
    uint32_t passed;
    uint32_t failed;
    uint32_t skipped;
} test_suite_t;

// Test statistics
typedef struct {
    uint32_t total_tests;
    uint32_t total_passed;
    uint32_t total_failed;
    uint32_t total_skipped;
    uint32_t total_suites;
} test_stats_t;

// Assertion macros
#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printk("ASSERT FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            printk("ASSERT_EQ FAILED: %s != %s (%d != %d) at %s:%d\n", \
                   #a, #b, (int)(a), (int)(b), __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            printk("ASSERT_NE FAILED: %s == %s at %s:%d\n", \
                   #a, #b, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            printk("ASSERT_NULL FAILED: %s is not NULL at %s:%d\n", \
                   #ptr, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            printk("ASSERT_NOT_NULL FAILED: %s is NULL at %s:%d\n", \
                   #ptr, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

// Function declarations
void test_framework_init(void);
void test_register_suite(test_suite_t* suite);
void test_run_all(void);
void test_run_suite(const char* suite_name);
test_stats_t* test_get_stats(void);
void test_report(void);

// Test helper functions
void test_print_banner(const char* text);
void test_print_pass(const char* test_name);
void test_print_fail(const char* test_name, const char* reason);
void test_print_skip(const char* test_name, const char* reason);

#endif // TEST_FRAMEWORK_H
