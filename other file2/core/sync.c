// ============================================================================
// Synchronization Primitives
// Spinlocks, Mutexes, Semaphores для многозадачности
// ============================================================================

#include "../include/sync.h"
#include "../include/process.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void schedule(void);

// ============================================================================
// Atomic operations (inline assembly)
// ============================================================================

// Atomic compare-and-swap
static inline bool atomic_cas(volatile uint32_t* ptr, uint32_t expected, uint32_t desired) {
    uint32_t prev;
    __asm__ volatile(
        "lock cmpxchgl %2, %1"
        : "=a"(prev), "+m"(*ptr)
        : "r"(desired), "0"(expected)
        : "memory"
    );
    return prev == expected;
}

// Atomic increment
static inline void atomic_inc(volatile int32_t* ptr) {
    __asm__ volatile(
        "lock incl %0"
        : "+m"(*ptr)
        :
        : "memory"
    );
}

// Atomic decrement
static inline void atomic_dec(volatile int32_t* ptr) {
    __asm__ volatile(
        "lock decl %0"
        : "+m"(*ptr)
        :
        : "memory"
    );
}

// ============================================================================
// Spinlock Implementation
// ============================================================================

void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

void spinlock_acquire(spinlock_t* lock) {
    // Busy-wait until we can acquire the lock
    while (!atomic_cas(&lock->locked, 0, 1)) {
        // Spin (pause for performance)
        __asm__ volatile("pause");
    }
}

void spinlock_release(spinlock_t* lock) {
    // Release the lock
    __asm__ volatile("" ::: "memory");  // Memory barrier
    lock->locked = 0;
}

bool spinlock_try_acquire(spinlock_t* lock) {
    return atomic_cas(&lock->locked, 0, 1);
}

// ============================================================================
// Mutex Implementation
// ============================================================================

void mutex_init(mutex_t* mutex) {
    mutex->locked = 0;
    mutex->owner_pid = 0;
}

void mutex_lock(mutex_t* mutex) {
    process_t* current = process_get_current();
    uint32_t pid = current ? current->pid : 0;

    // Try to acquire
    while (!atomic_cas(&mutex->locked, 0, 1)) {
        // Failed to acquire, yield CPU to other processes
        schedule();
    }

    // Lock acquired, set owner
    mutex->owner_pid = pid;
}

void mutex_unlock(mutex_t* mutex) {
    process_t* current = process_get_current();
    uint32_t pid = current ? current->pid : 0;

    // Check if this process owns the mutex
    if (mutex->owner_pid != pid) {
        terminal_write_line("[MUTEX] ERROR: Attempt to unlock mutex not owned by this process!");
        return;
    }

    // Clear owner and release lock
    mutex->owner_pid = 0;
    __asm__ volatile("" ::: "memory");  // Memory barrier
    mutex->locked = 0;
}

bool mutex_try_lock(mutex_t* mutex) {
    process_t* current = process_get_current();
    uint32_t pid = current ? current->pid : 0;

    if (atomic_cas(&mutex->locked, 0, 1)) {
        mutex->owner_pid = pid;
        return true;
    }
    return false;
}

// ============================================================================
// Semaphore Implementation
// ============================================================================

void semaphore_init(semaphore_t* sem, int32_t initial_value) {
    sem->value = initial_value;
    spinlock_init(&sem->lock);
}

void semaphore_wait(semaphore_t* sem) {
    while (1) {
        spinlock_acquire(&sem->lock);

        if (sem->value > 0) {
            sem->value--;
            spinlock_release(&sem->lock);
            return;
        }

        spinlock_release(&sem->lock);

        // Value is 0, yield CPU
        schedule();
    }
}

void semaphore_signal(semaphore_t* sem) {
    spinlock_acquire(&sem->lock);
    sem->value++;
    spinlock_release(&sem->lock);
}

bool semaphore_try_wait(semaphore_t* sem) {
    spinlock_acquire(&sem->lock);

    if (sem->value > 0) {
        sem->value--;
        spinlock_release(&sem->lock);
        return true;
    }

    spinlock_release(&sem->lock);
    return false;
}

int32_t semaphore_get_value(semaphore_t* sem) {
    spinlock_acquire(&sem->lock);
    int32_t value = sem->value;
    spinlock_release(&sem->lock);
    return value;
}
