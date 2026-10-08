// ============================================================================
// Performance Profiler
// System performance monitoring and profiling
// ============================================================================

#include "../include/profiler.h"

static profiler_stats_t stats;
static profile_entry_t* profile_entries = NULL;
static int profile_count = 0;

// ============================================================================
// Initialization
// ============================================================================

void profiler_init(void) {
    stats.total_syscalls = 0;
    stats.total_interrupts = 0;
    stats.total_context_switches = 0;
    stats.memory_allocated = 0;
    stats.memory_freed = 0;
    stats.cpu_usage = 0;
    stats.uptime_ms = 0;

    profile_count = 0;
}

// ============================================================================
// Event Recording
// ============================================================================

void profiler_record_syscall(uint32_t syscall_num) {
    stats.total_syscalls++;
}

void profiler_record_interrupt(uint32_t irq) {
    stats.total_interrupts++;
}

void profiler_record_context_switch(void) {
    stats.total_context_switches++;
}

void profiler_record_alloc(uint32_t size) {
    stats.memory_allocated += size;
}

void profiler_record_free(uint32_t size) {
    stats.memory_freed += size;
}

// ============================================================================
// Profiling Functions
// ============================================================================

void profiler_start(const char* name) {
    if (!name || profile_count >= 64) return;

    profile_entry_t* entry = &profile_entries[profile_count];

    // Copy name
    int i;
    for (i = 0; i < 31 && name[i]; i++) {
        entry->name[i] = name[i];
    }
    entry->name[i] = '\0';

    // TODO: Get current time
    entry->start_time = 0;
    entry->end_time = 0;
    entry->duration = 0;

    profile_count++;
}

void profiler_end(const char* name) {
    if (!name) return;

    for (int i = 0; i < profile_count; i++) {
        int match = 1;
        for (int j = 0; j < 32; j++) {
            if (profile_entries[i].name[j] != name[j]) {
                match = 0;
                break;
            }
            if (name[j] == '\0') break;
        }

        if (match) {
            // TODO: Get current time
            profile_entries[i].end_time = 0;
            profile_entries[i].duration = profile_entries[i].end_time - profile_entries[i].start_time;
            return;
        }
    }
}

// ============================================================================
// Statistics
// ============================================================================

void profiler_get_stats(profiler_stats_t* out_stats) {
    if (!out_stats) return;
    *out_stats = stats;
}

void profiler_update_cpu_usage(uint32_t usage_percent) {
    stats.cpu_usage = usage_percent;
}

void profiler_update_uptime(uint32_t uptime_ms) {
    stats.uptime_ms = uptime_ms;
}
