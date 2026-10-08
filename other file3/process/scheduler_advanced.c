// ============================================================================
// Advanced Scheduler
// Multiple scheduling algorithms and load balancing
// ============================================================================

#include "../include/scheduler.h"
#include "../include/smp.h"
#include "../include/heap.h"

// Scheduler statistics
static sched_stats_t stats;
static cpu_load_t cpu_loads[8];

// ============================================================================
// Initialization
// ============================================================================

void scheduler_init_advanced(void) {
    stats.total_switches = 0;
    stats.total_migrations = 0;
    stats.current_tasks = 0;
    stats.load_avg_1min = 0;
    stats.load_avg_5min = 0;

    int cpu_count = smp_get_cpu_count();
    for (int i = 0; i < cpu_count; i++) {
        cpu_loads[i].cpu_id = i;
        cpu_loads[i].load_average = 0;
        cpu_loads[i].tasks_running = 0;
        cpu_loads[i].idle_time = 0;
    }
}

// ============================================================================
// Scheduling Policy
// ============================================================================

void scheduler_set_policy(thread_t* thread, uint8_t policy) {
    if (!thread) return;

    // TODO: Implement policy change
    // For now, all threads use SCHED_NORMAL
}

void scheduler_set_priority(thread_t* thread, uint8_t priority) {
    if (!thread) return;

    if (priority > PRIO_MAX) {
        priority = PRIO_MAX;
    }

    thread->priority = priority;
}

// ============================================================================
// Load Balancing
// ============================================================================

void scheduler_balance_load(void) {
    int cpu_count = smp_get_cpu_count();

    // Calculate load per CPU
    uint32_t total_load = 0;
    for (int i = 0; i < cpu_count; i++) {
        cpu_loads[i].load_average = cpu_loads[i].tasks_running * 100;
        total_load += cpu_loads[i].load_average;
    }

    // Find most and least loaded CPUs
    int max_cpu = 0, min_cpu = 0;
    uint32_t max_load = cpu_loads[0].load_average;
    uint32_t min_load = cpu_loads[0].load_average;

    for (int i = 1; i < cpu_count; i++) {
        if (cpu_loads[i].load_average > max_load) {
            max_load = cpu_loads[i].load_average;
            max_cpu = i;
        }
        if (cpu_loads[i].load_average < min_load) {
            min_load = cpu_loads[i].load_average;
            min_cpu = i;
        }
    }

    // Balance if difference > threshold (50%)
    if (max_load > min_load + 50) {
        // TODO: Migrate thread from max_cpu to min_cpu
        stats.total_migrations++;
    }
}

// ============================================================================
// Statistics
// ============================================================================

cpu_load_t* scheduler_get_cpu_load(uint8_t cpu_id) {
    if (cpu_id >= smp_get_cpu_count()) {
        return NULL;
    }
    return &cpu_loads[cpu_id];
}

void scheduler_get_stats(sched_stats_t* out_stats) {
    if (!out_stats) return;
    *out_stats = stats;
}
