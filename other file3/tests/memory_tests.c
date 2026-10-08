#include "test_framework.h"
#include "../include/common.h"
#include "../include/pmm.h"
#include "../include/paging.h"
#include "../include/heap.h"

// Test: PMM allocation and free
static bool test_pmm_alloc_free(void) {
    void* page1 = pmm_alloc();
    ASSERT_NOT_NULL(page1);

    void* page2 = pmm_alloc();
    ASSERT_NOT_NULL(page2);
    ASSERT_NE(page1, page2);

    pmm_free(page1);
    pmm_free(page2);

    return true;
}

// Test: Multiple allocations
static bool test_pmm_multiple_alloc(void) {
    #define NUM_PAGES 10
    void* pages[NUM_PAGES];

    // Allocate pages
    for (int i = 0; i < NUM_PAGES; i++) {
        pages[i] = pmm_alloc();
        ASSERT_NOT_NULL(pages[i]);
    }

    // Verify all different
    for (int i = 0; i < NUM_PAGES; i++) {
        for (int j = i + 1; j < NUM_PAGES; j++) {
            ASSERT_NE(pages[i], pages[j]);
        }
    }

    // Free pages
    for (int i = 0; i < NUM_PAGES; i++) {
        pmm_free(pages[i]);
    }

    return true;
}

// Test: Heap allocation
static bool test_heap_alloc(void) {
    void* ptr1 = kmalloc(64);
    ASSERT_NOT_NULL(ptr1);

    void* ptr2 = kmalloc(128);
    ASSERT_NOT_NULL(ptr2);

    kfree(ptr1);
    kfree(ptr2);

    return true;
}

// Test: Heap reallocation
static bool test_heap_realloc(void) {
    void* ptr = kmalloc(64);
    ASSERT_NOT_NULL(ptr);

    // Write pattern
    for (int i = 0; i < 64; i++) {
        ((uint8_t*)ptr)[i] = (uint8_t)i;
    }

    // Reallocate
    void* new_ptr = krealloc(ptr, 128);
    ASSERT_NOT_NULL(new_ptr);

    // Verify pattern preserved
    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(((uint8_t*)new_ptr)[i], (uint8_t)i);
    }

    kfree(new_ptr);
    return true;
}

// Test: Heap stress test
static bool test_heap_stress(void) {
    #define NUM_ALLOCS 100
    void* ptrs[NUM_ALLOCS];

    // Allocate random sizes
    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t size = (i % 16 + 1) * 64;
        ptrs[i] = kmalloc(size);
        ASSERT_NOT_NULL(ptrs[i]);
    }

    // Free in reverse order
    for (int i = NUM_ALLOCS - 1; i >= 0; i--) {
        kfree(ptrs[i]);
    }

    return true;
}

// Test: Zero allocation
static bool test_heap_zero_alloc(void) {
    void* ptr = kmalloc(0);
    // Zero allocation may return NULL or a valid pointer
    // Both behaviors are acceptable
    if (ptr) {
        kfree(ptr);
    }
    return true;
}

// Test: Large allocation
static bool test_heap_large_alloc(void) {
    void* ptr = kmalloc(4096);
    ASSERT_NOT_NULL(ptr);

    // Write and verify
    memset(ptr, 0xAA, 4096);
    for (int i = 0; i < 4096; i++) {
        ASSERT_EQ(((uint8_t*)ptr)[i], 0xAA);
    }

    kfree(ptr);
    return true;
}

// Test: Paging setup
static bool test_paging_setup(void) {
    // Paging should already be enabled
    // Just verify we can access memory
    volatile uint32_t* test_ptr = (uint32_t*)0xC0000000;
    *test_ptr = 0x12345678;
    ASSERT_EQ(*test_ptr, 0x12345678);
    return true;
}

// Test case array
static test_case_t memory_test_cases[] = {
    {"PMM Alloc/Free", "Test physical memory allocation", test_pmm_alloc_free},
    {"PMM Multiple Alloc", "Test multiple allocations", test_pmm_multiple_alloc},
    {"Heap Alloc", "Test heap allocation", test_heap_alloc},
    {"Heap Realloc", "Test heap reallocation", test_heap_realloc},
    {"Heap Stress", "Stress test heap allocator", test_heap_stress},
    {"Heap Zero Alloc", "Test zero-size allocation", test_heap_zero_alloc},
    {"Heap Large Alloc", "Test large allocation", test_heap_large_alloc},
    {"Paging Setup", "Test paging functionality", test_paging_setup},
};

// Test suite definition
static test_suite_t memory_test_suite = {
    .name = "Memory Tests",
    .tests = memory_test_cases,
    .test_count = sizeof(memory_test_cases) / sizeof(test_case_t),
};

// Register suite
void memory_tests_register(void) {
    test_register_suite(&memory_test_suite);
}
