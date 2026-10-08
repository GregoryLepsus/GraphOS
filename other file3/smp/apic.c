// ============================================================================
// Local APIC (Advanced Programmable Interrupt Controller)
// Multi-core CPU interrupt management
// ============================================================================

#include "../include/apic.h"
#include "../include/smp.h"

// APIC base address (detected from MSR or hardcoded)
static volatile uint32_t* apic_base = (uint32_t*)0xFEE00000;

// ============================================================================
// APIC Register Access
// ============================================================================

static inline uint32_t apic_read(uint32_t reg) {
    return apic_base[reg >> 2];
}

static inline void apic_write(uint32_t reg, uint32_t value) {
    apic_base[reg >> 2] = value;
}

// ============================================================================
// APIC Initialization
// ============================================================================

void apic_init(void) {
    // Enable APIC via Spurious Interrupt Vector Register
    uint32_t svr = apic_read(APIC_SVR);
    svr |= 0x100;  // APIC Software Enable
    apic_write(APIC_SVR, svr | 0xFF);  // Spurious vector = 0xFF

    // Clear task priority (allow all interrupts)
    apic_write(APIC_TPR, 0);
}

void apic_enable(void) {
    // Enable APIC in MSR
    uint32_t eax, edx;
    __asm__ volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x1B));
    eax |= 0x800;  // APIC global enable
    __asm__ volatile("wrmsr" : : "a"(eax), "d"(edx), "c"(0x1B));
}

// ============================================================================
// APIC Information
// ============================================================================

uint32_t apic_get_id(void) {
    return (apic_read(APIC_ID) >> 24) & 0xFF;
}

// ============================================================================
// End of Interrupt
// ============================================================================

void apic_eoi(void) {
    apic_write(APIC_EOI, 0);
}

// ============================================================================
// Inter-Processor Interrupts (IPI)
// ============================================================================

void apic_send_ipi(uint8_t dest_apic_id, uint8_t vector) {
    // Wait for idle
    while (apic_read(APIC_ICR_LOW) & (1 << 12));

    // Set destination
    apic_write(APIC_ICR_HIGH, ((uint32_t)dest_apic_id) << 24);

    // Send IPI
    apic_write(APIC_ICR_LOW, vector | APIC_ICR_FIXED);
}

void apic_send_init(uint8_t dest_apic_id) {
    // Wait for idle
    while (apic_read(APIC_ICR_LOW) & (1 << 12));

    // Set destination
    apic_write(APIC_ICR_HIGH, ((uint32_t)dest_apic_id) << 24);

    // Send INIT IPI
    apic_write(APIC_ICR_LOW, APIC_ICR_INIT | (1 << 14));  // Assert
}

void apic_send_startup(uint8_t dest_apic_id, uint8_t vector) {
    // Wait for idle
    while (apic_read(APIC_ICR_LOW) & (1 << 12));

    // Set destination
    apic_write(APIC_ICR_HIGH, ((uint32_t)dest_apic_id) << 24);

    // Send SIPI
    apic_write(APIC_ICR_LOW, vector | APIC_ICR_STARTUP);
}
