// ============================================================================
// Threading System
// Kernel-level thread support
// ============================================================================

#include "../include/thread.h"
#include "../include/heap.h"
#include "../include/process.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);

// Thread list
static thread_t* thread_list = NULL;
static thread_t* current_thread = NULL;
static uint32_t next_tid = 1;

// ============================================================================
// Thread Initialization
// ============================================================================

void thread_init(void) {
    terminal_write_line("[Thread] Initializing threading system...");

    // Create main thread for current process
    thread_t* main = (thread_t*)kmalloc(sizeof(thread_t));
    main->tid = 0;
    main->pid = 0;  // Kernel process
    main->state = THREAD_STATE_RUNNING;
    main->priority = 4;
    main->cpu_affinity = 0;
    main->kernel_stack = NULL;  // Using current stack
    main->user_stack = NULL;
    main->stack_size = 0;
    main->tls = NULL;
    main->wait_object = NULL;
    main->sleep_until = 0;
    main->exit_status = 0;
    main->next = NULL;
    main->prev = NULL;

    thread_list = main;
    current_thread = main;

    terminal_write_line("[Thread] Threading system initialized");
}

// ============================================================================
// Thread Creation
// ============================================================================

thread_t* thread_create(void* entry_point, void* arg, uint32_t priority) {
    thread_t* thread = (thread_t*)kmalloc(sizeof(thread_t));

    thread->tid = next_tid++;
    thread->pid = 0;  // TODO: Get current process PID
    thread->state = THREAD_STATE_READY;
    thread->priority = priority;
    thread->cpu_affinity = 0xFF;  // Any CPU

    // Allocate kernel stack
    thread->stack_size = 8192;  // 8KB
    thread->kernel_stack = kmalloc(thread->stack_size);
    thread->user_stack = NULL;

    // Setup stack with entry point
    uint32_t* stack_ptr = (uint32_t*)((uint32_t)thread->kernel_stack + thread->stack_size);
    stack_ptr--;
    *stack_ptr = (uint32_t)arg;       // Argument
    stack_ptr--;
    *stack_ptr = (uint32_t)entry_point;  // Return address

    thread->context.esp = (uint32_t)stack_ptr;
    thread->context.ebp = (uint32_t)stack_ptr;

    thread->tls = NULL;
    thread->wait_object = NULL;
    thread->sleep_until = 0;
    thread->exit_status = 0;

    // Add to thread list
    thread->next = thread_list;
    thread->prev = NULL;
    if (thread_list) {
        thread_list->prev = thread;
    }
    thread_list = thread;

    return thread;
}

thread_t* thread_create_kernel(void* entry_point, void* arg) {
    return thread_create(entry_point, arg, 4);  // Default priority
}

// ============================================================================
// Thread Control
// ============================================================================

void thread_exit(int status) {
    if (!current_thread) {
        return;
    }

    current_thread->state = THREAD_STATE_DEAD;
    current_thread->exit_status = status;

    // TODO: Free resources
    // TODO: Schedule next thread
}

void thread_yield(void) {
    // TODO: Switch to next ready thread
}

int thread_join(thread_t* thread, int* status) {
    if (!thread) {
        return -1;
    }

    // Wait for thread to finish
    while (thread->state != THREAD_STATE_DEAD) {
        thread_yield();
    }

    if (status) {
        *status = thread->exit_status;
    }

    return 0;
}

void thread_sleep(uint32_t ms) {
    if (!current_thread) {
        return;
    }

    // TODO: Set sleep_until timestamp
    current_thread->state = THREAD_STATE_BLOCKED;
    thread_yield();
}

// ============================================================================
// Thread Query
// ============================================================================

thread_t* thread_get_current(void) {
    return current_thread;
}

void thread_set_affinity(thread_t* thread, uint8_t cpu_id) {
    if (thread) {
        thread->cpu_affinity = cpu_id;
    }
}
