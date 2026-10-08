// ============================================================================
// Memory Optimization
// Memory compaction and optimization strategies
// ============================================================================

#include "../include/memory_optimizer.h"
#include "../include/heap.h"
#include "../include/pmm.h"

static mem_stats_t mem_stats;

// ============================================================================
// Initialization
// ============================================================================

void memopt_init(void) {
    mem_stats.pages_allocated = 0;
    mem_stats.pages_freed = 0;
    mem_stats.heap_allocated = 0;
    mem_stats.heap_freed = 0;
    mem_stats.fragmentation = 0;
    mem_stats.compaction_count = 0;
}

// ============================================================================
// Memory Statistics
// ============================================================================

void memopt_update_stats(void) {
    // Update statistics from PMM and heap
    pmm_info_t pmm_info;
    pmm_get_info(&pmm_info);

    mem_stats.pages_allocated = pmm_info.used_pages;
    mem_stats.pages_freed = pmm_info.total_pages - pmm_info.used_pages;

    // Calculate fragmentation percentage
    if (pmm_info.total_pages > 0) {
        mem_stats.fragmentation = (pmm_info.used_pages * 100) / pmm_info.total_pages;
    }
}

void memopt_get_stats(mem_stats_t* stats) {
    if (!stats) return;
    *stats = mem_stats;
}

// ============================================================================
// Memory Compaction
// ============================================================================

int memopt_compact_heap(void) {
    // TODO: Implement heap compaction
    // Move allocated blocks together to reduce fragmentation
    mem_stats.compaction_count++;
    return 0;
}

// ============================================================================
// Memory Optimization
// ============================================================================

void memopt_optimize(void) {
    memopt_update_stats();

    // If fragmentation > 70%, compact
    if (mem_stats.fragmentation > 70) {
        memopt_compact_heap();
    }
}

// ============================================================================
// Memory Pressure Detection
// ============================================================================

int memopt_is_low_memory(void) {
    pmm_info_t pmm_info;
    pmm_get_info(&pmm_info);

    uint32_t free_pages = pmm_info.total_pages - pmm_info.used_pages;
    uint32_t threshold = pmm_info.total_pages / 10;  // 10% threshold

    return free_pages < threshold;
}
