// ============================================================================
// SMP (Symmetric Multiprocessing) Support
// Multi-core CPU initialization and management
// ============================================================================

#include "../include/smp.h"
#include "../include/apic.h"
#include "../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// CPU information table
static cpu_info_t cpus[8];
static int cpu_count = 0;
static int bsp_id = 0;

// ============================================================================
// SMP Initialization
// ============================================================================

void smp_init(void) {
    terminal_write_line("[SMP] Initializing multiprocessing...");

    // Initialize BSP (Bootstrap Processor)
    apic_init();
    apic_enable();

    bsp_id = apic_get_id();

    cpus[0].cpu_id = 0;
    cpus[0].apic_id = bsp_id;
    cpus[0].state = CPU_STATE_ONLINE;
    cpus[0].flags = 0;
    cpus[0].stack = NULL;  // Using current stack
    cpus[0].tss = NULL;

    cpu_count = 1;

    terminal_write("[SMP] BSP APIC ID: ");
    terminal_write_hex(bsp_id);
    terminal_write_line("");

    // TODO: Detect and start APs (Application Processors)
    // This requires ACPI MADT parsing

    terminal_write("[SMP] CPU count: ");
    terminal_write_hex(cpu_count);
    terminal_write_line("");
}

// ============================================================================
// CPU Information
// ============================================================================

int smp_get_cpu_count(void) {
    return cpu_count;
}

cpu_info_t* smp_get_current_cpu(void) {
    uint8_t apic_id = apic_get_id();

    for (int i = 0; i < cpu_count; i++) {
        if (cpus[i].apic_id == apic_id) {
            return &cpus[i];
        }
    }

    return &cpus[0];  // Fallback to BSP
}

cpu_info_t* smp_get_cpu(uint8_t cpu_id) {
    if (cpu_id >= cpu_count) {
        return NULL;
    }
    return &cpus[cpu_id];
}

// ============================================================================
// AP Startup
// ============================================================================

void smp_start_ap(uint8_t apic_id) {
    if (cpu_count >= 8) {
        return;  // Max CPUs reached
    }

    // Allocate stack for AP
    void* stack = kmalloc(8192);  // 8KB stack

    // Setup CPU info
    cpus[cpu_count].cpu_id = cpu_count;
    cpus[cpu_count].apic_id = apic_id;
    cpus[cpu_count].state = CPU_STATE_OFFLINE;
    cpus[cpu_count].flags = 0;
    cpus[cpu_count].stack = stack;
    cpus[cpu_count].tss = NULL;

    // Send INIT IPI
    apic_send_init(apic_id);

    // Wait 10ms
    for (volatile int i = 0; i < 100000; i++);

    // Send SIPI (Startup IPI)
    apic_send_startup(apic_id, 0x08);  // Start at 0x8000

    // Wait 200us
    for (volatile int i = 0; i < 2000; i++);

    // Send second SIPI
    apic_send_startup(apic_id, 0x08);

    cpus[cpu_count].state = CPU_STATE_ONLINE;
    cpu_count++;
}

// ============================================================================
// IPI Functions
// ============================================================================

void smp_send_ipi(uint8_t cpu_id, uint8_t vector) {
    if (cpu_id >= cpu_count) {
        return;
    }

    apic_send_ipi(cpus[cpu_id].apic_id, vector);
}

void smp_broadcast_ipi(uint8_t vector) {
    for (int i = 0; i < cpu_count; i++) {
        if (cpus[i].cpu_id != smp_get_current_cpu()->cpu_id) {
            apic_send_ipi(cpus[i].apic_id, vector);
        }
    }
}
