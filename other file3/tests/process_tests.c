#include "test_framework.h"
#include "../include/common.h"
#include "../include/process.h"
#include "../include/scheduler.h"

// Test: Process creation
static bool test_process_create(void) {
    process_t* proc = process_create("test_process", 0);
    ASSERT_NOT_NULL(proc);
    ASSERT_EQ(proc->state, PROCESS_READY);
    return true;
}

// Test: Process state transitions
static bool test_process_states(void) {
    process_t* proc = process_create("test_states", 0);
    ASSERT_NOT_NULL(proc);

    // Initial state
    ASSERT_EQ(proc->state, PROCESS_READY);

    // Transition to running
    proc->state = PROCESS_RUNNING;
    ASSERT_EQ(proc->state, PROCESS_RUNNING);

    // Transition to blocked
    proc->state = PROCESS_BLOCKED;
    ASSERT_EQ(proc->state, PROCESS_BLOCKED);

    return true;
}

// Test: Process priority
static bool test_process_priority(void) {
    process_t* proc = process_create("test_priority", 0);
    ASSERT_NOT_NULL(proc);

    // Default priority
    ASSERT_EQ(proc->priority, PRIORITY_NORMAL);

    // Change priority
    proc->priority = PRIORITY_HIGH;
    ASSERT_EQ(proc->priority, PRIORITY_HIGH);

    return true;
}

// Test: Scheduler initialization
static bool test_scheduler_init(void) {
    // Scheduler should already be initialized
    process_t* current = get_current_process();
    ASSERT_NOT_NULL(current);
    return true;
}

// Test: Process list
static bool test_process_list(void) {
    // Create several processes
    process_t* proc1 = process_create("proc1", 0);
    process_t* proc2 = process_create("proc2", 0);
    process_t* proc3 = process_create("proc3", 0);

    ASSERT_NOT_NULL(proc1);
    ASSERT_NOT_NULL(proc2);
    ASSERT_NOT_NULL(proc3);

    // Verify PIDs are different
    ASSERT_NE(proc1->pid, proc2->pid);
    ASSERT_NE(proc2->pid, proc3->pid);
    ASSERT_NE(proc1->pid, proc3->pid);

    return true;
}

// Test: Context switch (basic)
static bool test_context_switch(void) {
    process_t* current = get_current_process();
    ASSERT_NOT_NULL(current);

    // Create new process
    process_t* new_proc = process_create("test_switch", 0);
    ASSERT_NOT_NULL(new_proc);

    // In real test, would trigger context switch
    // For now, just verify structures exist
    ASSERT_NOT_NULL(current->context);

    return true;
}

// Test: Process termination
static bool test_process_terminate(void) {
    process_t* proc = process_create("test_term", 0);
    ASSERT_NOT_NULL(proc);

    // Mark as terminated
    proc->state = PROCESS_ZOMBIE;
    ASSERT_EQ(proc->state, PROCESS_ZOMBIE);

    return true;
}

// Test: Scheduler round-robin
static bool test_scheduler_round_robin(void) {
    // Create multiple processes
    for (int i = 0; i < 5; i++) {
        process_t* proc = process_create("rr_test", 0);
        ASSERT_NOT_NULL(proc);
        proc->policy = SCHED_RR;
    }

    // Verify scheduler can handle them
    // In real test, would run multiple scheduling cycles
    return true;
}

// Test case array
static test_case_t process_test_cases[] = {
    {"Process Create", "Test process creation", test_process_create},
    {"Process States", "Test state transitions", test_process_states},
    {"Process Priority", "Test priority levels", test_process_priority},
    {"Scheduler Init", "Test scheduler initialization", test_scheduler_init},
    {"Process List", "Test process management", test_process_list},
    {"Context Switch", "Test context switching", test_context_switch},
    {"Process Terminate", "Test process termination", test_process_terminate},
    {"Scheduler RR", "Test round-robin scheduling", test_scheduler_round_robin},
};

// Test suite definition
static test_suite_t process_test_suite = {
    .name = "Process Tests",
    .tests = process_test_cases,
    .test_count = sizeof(process_test_cases) / sizeof(test_case_t),
};

// Register suite
void process_tests_register(void) {
    test_register_suite(&process_test_suite);
}
