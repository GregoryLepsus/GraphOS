// ============================================================================
// Advanced Timer Management
// High-resolution timers and timer callbacks
// ============================================================================

#include "../include/timer_advanced.h"
#include "../include/heap.h"

static timer_entry_t* timers = NULL;
static uint32_t next_timer_id = 1;

// ============================================================================
// Initialization
// ============================================================================

void timer_advanced_init(void) {
    timers = NULL;
}

// ============================================================================
// Timer Creation
// ============================================================================

uint32_t timer_create(timer_callback_t callback, void* data, uint32_t interval_ms, uint8_t flags) {
    if (!callback) return 0;

    timer_entry_t* timer = (timer_entry_t*)kmalloc(sizeof(timer_entry_t));
    if (!timer) return 0;

    timer->id = next_timer_id++;
    timer->callback = callback;
    timer->data = data;
    timer->interval_ms = interval_ms;
    timer->expires_at = 0;  // TODO: Get current time + interval
    timer->flags = flags;
    timer->next = timers;

    timers = timer;

    return timer->id;
}

int timer_delete(uint32_t timer_id) {
    timer_entry_t* prev = NULL;
    timer_entry_t* curr = timers;

    while (curr) {
        if (curr->id == timer_id) {
            if (prev) {
                prev->next = curr->next;
            } else {
                timers = curr->next;
            }
            kfree(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    return -1;
}

// ============================================================================
// Timer Processing
// ============================================================================

void timer_tick(void) {
    uint32_t current_time = 0;  // TODO: Get current time

    timer_entry_t* curr = timers;

    while (curr) {
        if (current_time >= curr->expires_at) {
            // Fire callback
            curr->callback(curr->data);

            // Reschedule if periodic
            if (curr->flags & TIMER_PERIODIC) {
                curr->expires_at = current_time + curr->interval_ms;
            } else {
                // One-shot - mark for deletion
                curr->flags |= 0x80;
            }
        }
        curr = curr->next;
    }

    // Clean up one-shot timers
    timer_entry_t* prev = NULL;
    curr = timers;

    while (curr) {
        if (curr->flags & 0x80) {
            timer_entry_t* next = curr->next;
            if (prev) {
                prev->next = next;
            } else {
                timers = next;
            }
            kfree(curr);
            curr = next;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

// ============================================================================
// High-Resolution Sleep
// ============================================================================

void nanosleep(uint32_t nanoseconds) {
    // TODO: Implement high-resolution sleep
    // For now, busy wait
    for (volatile uint32_t i = 0; i < nanoseconds / 100; i++);
}
