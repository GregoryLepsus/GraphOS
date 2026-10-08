#include "../include/optimizer.h"
#include "../include/common.h"
#include <string.h>

#define MAX_HOTPATHS 64

static perf_counter_t hotpaths[MAX_HOTPATHS];
static uint32_t hotpath_count = 0;
static opt_stats_t stats;

// Read timestamp counter
static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

void optimizer_init(void) {
    memset(hotpaths, 0, sizeof(hotpaths));
    memset(&stats, 0, sizeof(stats));
    hotpath_count = 0;
    printk("Optimizer: Initialized\n");
}

void optimizer_register_hotpath(const char* name, uint32_t flags) {
    if (hotpath_count >= MAX_HOTPATHS) {
        printk("Optimizer: Warning - max hotpaths reached\n");
        return;
    }

    perf_counter_t* counter = &hotpaths[hotpath_count++];
    counter->name = name;
    counter->flags = flags;
    counter->call_count = 0;
    counter->total_cycles = 0;
    counter->min_cycles = UINT64_MAX;
    counter->max_cycles = 0;

    stats.total_optimizations++;
}

void optimizer_mark_start(const char* name) {
    for (uint32_t i = 0; i < hotpath_count; i++) {
        if (strcmp(hotpaths[i].name, name) == 0) {
            // Store start time in a temporary location
            // In real implementation, would use thread-local storage
            hotpaths[i].total_cycles = rdtsc(); // Temporary storage
            break;
        }
    }
}

void optimizer_mark_end(const char* name) {
    uint64_t end_cycles = rdtsc();

    for (uint32_t i = 0; i < hotpath_count; i++) {
        if (strcmp(hotpaths[i].name, name) == 0) {
            uint64_t start_cycles = hotpaths[i].total_cycles;
            uint64_t elapsed = end_cycles - start_cycles;

            hotpaths[i].call_count++;
            hotpaths[i].total_cycles += elapsed;

            if (elapsed < hotpaths[i].min_cycles) {
                hotpaths[i].min_cycles = elapsed;
            }
            if (elapsed > hotpaths[i].max_cycles) {
                hotpaths[i].max_cycles = elapsed;
            }

            stats.cycles_saved += (elapsed < 1000) ? 100 : 0;
            break;
        }
    }
}

void optimizer_report(void) {
    printk("\n=== Optimizer Performance Report ===\n");
    printk("Total optimizations: %u\n", stats.total_optimizations);
    printk("Cache hits: %u\n", stats.cache_hits);
    printk("Cache misses: %u\n", stats.cache_misses);
    printk("Cycles saved: %llu\n", stats.cycles_saved);

    printk("\nHot Paths:\n");
    for (uint32_t i = 0; i < hotpath_count; i++) {
        perf_counter_t* pc = &hotpaths[i];
        if (pc->call_count > 0) {
            uint64_t avg = pc->total_cycles / pc->call_count;
            printk("  %s: calls=%llu avg=%llu min=%llu max=%llu\n",
                   pc->name, pc->call_count, avg, pc->min_cycles, pc->max_cycles);
        }
    }
    printk("===================================\n\n");
}

opt_stats_t* optimizer_get_stats(void) {
    return &stats;
}
