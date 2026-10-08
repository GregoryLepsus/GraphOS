#include "test_framework.h"
#include "../include/common.h"
#include <string.h>

#define MAX_SUITES 32

static test_suite_t* suites[MAX_SUITES];
static uint32_t suite_count = 0;
static test_stats_t stats;

void test_framework_init(void) {
    memset(suites, 0, sizeof(suites));
    memset(&stats, 0, sizeof(stats));
    suite_count = 0;
    printk("Test Framework: Initialized\n");
}

void test_register_suite(test_suite_t* suite) {
    if (suite_count >= MAX_SUITES) {
        printk("Test Framework: Warning - max suites reached\n");
        return;
    }
    suites[suite_count++] = suite;
    stats.total_suites++;
    stats.total_tests += suite->test_count;
}

void test_print_banner(const char* text) {
    printk("\n");
    printk("========================================\n");
    printk("%s\n", text);
    printk("========================================\n");
}

void test_print_pass(const char* test_name) {
    printk("[PASS] %s\n", test_name);
}

void test_print_fail(const char* test_name, const char* reason) {
    printk("[FAIL] %s: %s\n", test_name, reason);
}

void test_print_skip(const char* test_name, const char* reason) {
    printk("[SKIP] %s: %s\n", test_name, reason);
}

static void run_test_case(test_case_t* test) {
    printk("Running: %s... ", test->name);

    // Run the test
    bool result = test->test_func();

    test->passed = result;
    test->skipped = false;

    if (result) {
        printk("PASS\n");
    } else {
        printk("FAIL\n");
    }
}

void test_run_suite(const char* suite_name) {
    for (uint32_t i = 0; i < suite_count; i++) {
        test_suite_t* suite = suites[i];
        if (strcmp(suite->name, suite_name) == 0) {
            test_print_banner(suite->name);

            suite->passed = 0;
            suite->failed = 0;
            suite->skipped = 0;

            for (uint32_t j = 0; j < suite->test_count; j++) {
                test_case_t* test = &suite->tests[j];
                run_test_case(test);

                if (test->passed) {
                    suite->passed++;
                    stats.total_passed++;
                } else if (test->skipped) {
                    suite->skipped++;
                    stats.total_skipped++;
                } else {
                    suite->failed++;
                    stats.total_failed++;
                }
            }

            printk("\nSuite Results: %u/%u passed, %u failed, %u skipped\n",
                   suite->passed, suite->test_count, suite->failed, suite->skipped);
            return;
        }
    }
    printk("Test suite '%s' not found\n", suite_name);
}

void test_run_all(void) {
    test_print_banner("Running All Test Suites");

    for (uint32_t i = 0; i < suite_count; i++) {
        test_run_suite(suites[i]->name);
    }

    test_report();
}

test_stats_t* test_get_stats(void) {
    return &stats;
}

void test_report(void) {
    test_print_banner("Test Report");

    printk("Total Suites: %u\n", stats.total_suites);
    printk("Total Tests:  %u\n", stats.total_tests);
    printk("Passed:       %u (%.1f%%)\n", stats.total_passed,
           (float)stats.total_passed * 100.0f / stats.total_tests);
    printk("Failed:       %u (%.1f%%)\n", stats.total_failed,
           (float)stats.total_failed * 100.0f / stats.total_tests);
    printk("Skipped:      %u (%.1f%%)\n", stats.total_skipped,
           (float)stats.total_skipped * 100.0f / stats.total_tests);

    if (stats.total_failed == 0) {
        printk("\nALL TESTS PASSED!\n");
    } else {
        printk("\nSOME TESTS FAILED!\n");
    }
}
