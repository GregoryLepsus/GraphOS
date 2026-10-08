// ============================================================================
// Advanced Synchronization Primitives
// Recursive mutexes, reader-writer locks, condition variables, barriers
// ============================================================================

#include "../include/sync_advanced.h"
#include "../include/thread.h"

// ============================================================================
// Recursive Mutex
// ============================================================================

void recursive_mutex_init(recursive_mutex_t* mutex) {
    if (!mutex) return;

    spinlock_init(&mutex->lock);
    mutex->owner_tid = 0;
    mutex->recursion_count = 0;
}

void recursive_mutex_lock(recursive_mutex_t* mutex) {
    if (!mutex) return;

    thread_t* current = thread_get_current();
    if (!current) return;

    spinlock_acquire(&mutex->lock);

    if (mutex->owner_tid == current->tid) {
        // Already own the lock, increment recursion
        mutex->recursion_count++;
        spinlock_release(&mutex->lock);
        return;
    }

    // Wait until lock is free
    while (mutex->owner_tid != 0) {
        spinlock_release(&mutex->lock);
        thread_yield();
        spinlock_acquire(&mutex->lock);
    }

    // Acquire lock
    mutex->owner_tid = current->tid;
    mutex->recursion_count = 1;
    spinlock_release(&mutex->lock);
}

int recursive_mutex_trylock(recursive_mutex_t* mutex) {
    if (!mutex) return -1;

    thread_t* current = thread_get_current();
    if (!current) return -1;

    spinlock_acquire(&mutex->lock);

    if (mutex->owner_tid == current->tid) {
        mutex->recursion_count++;
        spinlock_release(&mutex->lock);
        return 0;
    }

    if (mutex->owner_tid != 0) {
        spinlock_release(&mutex->lock);
        return -1;  // Already locked
    }

    mutex->owner_tid = current->tid;
    mutex->recursion_count = 1;
    spinlock_release(&mutex->lock);
    return 0;
}

void recursive_mutex_unlock(recursive_mutex_t* mutex) {
    if (!mutex) return;

    thread_t* current = thread_get_current();
    if (!current) return;

    spinlock_acquire(&mutex->lock);

    if (mutex->owner_tid != current->tid) {
        spinlock_release(&mutex->lock);
        return;  // Not owner
    }

    mutex->recursion_count--;

    if (mutex->recursion_count == 0) {
        mutex->owner_tid = 0;
    }

    spinlock_release(&mutex->lock);
}

// ============================================================================
// Reader-Writer Lock
// ============================================================================

void rwlock_init(rwlock_t* lock) {
    if (!lock) return;

    spinlock_init(&lock->lock);
    lock->readers = 0;
    lock->writers = 0;
    lock->write_waiting = 0;
    semaphore_init(&lock->read_sem, 1);
    semaphore_init(&lock->write_sem, 1);
}

void rwlock_read_lock(rwlock_t* lock) {
    if (!lock) return;

    spinlock_acquire(&lock->lock);

    // Wait if writer active or waiting
    while (lock->writers > 0 || lock->write_waiting > 0) {
        spinlock_release(&lock->lock);
        thread_yield();
        spinlock_acquire(&lock->lock);
    }

    lock->readers++;
    spinlock_release(&lock->lock);
}

void rwlock_write_lock(rwlock_t* lock) {
    if (!lock) return;

    spinlock_acquire(&lock->lock);
    lock->write_waiting++;

    // Wait until no readers and no writers
    while (lock->readers > 0 || lock->writers > 0) {
        spinlock_release(&lock->lock);
        thread_yield();
        spinlock_acquire(&lock->lock);
    }

    lock->write_waiting--;
    lock->writers = 1;
    spinlock_release(&lock->lock);
}

void rwlock_read_unlock(rwlock_t* lock) {
    if (!lock) return;

    spinlock_acquire(&lock->lock);
    lock->readers--;
    spinlock_release(&lock->lock);
}

void rwlock_write_unlock(rwlock_t* lock) {
    if (!lock) return;

    spinlock_acquire(&lock->lock);
    lock->writers = 0;
    spinlock_release(&lock->lock);
}

// ============================================================================
// Condition Variable
// ============================================================================

void condvar_init(condvar_t* cv) {
    if (!cv) return;

    spinlock_init(&cv->lock);
    cv->waiting_threads = NULL;
}

void condvar_wait(condvar_t* cv, mutex_t* mutex) {
    if (!cv || !mutex) return;

    thread_t* current = thread_get_current();
    if (!current) return;

    // Add to waiting list
    spinlock_acquire(&cv->lock);
    current->next = cv->waiting_threads;
    cv->waiting_threads = current;
    spinlock_release(&cv->lock);

    // Release mutex and block
    mutex_unlock(mutex);
    current->state = THREAD_STATE_BLOCKED;
    thread_yield();

    // Reacquire mutex
    mutex_lock(mutex);
}

void condvar_signal(condvar_t* cv) {
    if (!cv) return;

    spinlock_acquire(&cv->lock);

    if (cv->waiting_threads) {
        thread_t* thread = cv->waiting_threads;
        cv->waiting_threads = thread->next;
        thread->state = THREAD_STATE_READY;
        thread->next = NULL;
    }

    spinlock_release(&cv->lock);
}

void condvar_broadcast(condvar_t* cv) {
    if (!cv) return;

    spinlock_acquire(&cv->lock);

    while (cv->waiting_threads) {
        thread_t* thread = cv->waiting_threads;
        cv->waiting_threads = thread->next;
        thread->state = THREAD_STATE_READY;
        thread->next = NULL;
    }

    spinlock_release(&cv->lock);
}

// ============================================================================
// Barrier
// ============================================================================

void barrier_init(barrier_t* barrier, uint32_t threshold) {
    if (!barrier) return;

    barrier->threshold = threshold;
    barrier->count = 0;
    barrier->generation = 0;
    mutex_init(&barrier->lock);
    condvar_init(&barrier->cv);
}

void barrier_wait(barrier_t* barrier) {
    if (!barrier) return;

    mutex_lock(&barrier->lock);

    uint32_t gen = barrier->generation;
    barrier->count++;

    if (barrier->count >= barrier->threshold) {
        // Last thread - wake all
        barrier->generation++;
        barrier->count = 0;
        condvar_broadcast(&barrier->cv);
    } else {
        // Wait for others
        while (gen == barrier->generation) {
            condvar_wait(&barrier->cv, &barrier->lock);
        }
    }

    mutex_unlock(&barrier->lock);
}
