#ifndef APIC_H
#define APIC_H

#include <stdint.h>

// Local APIC Registers
#define APIC_ID              0x0020
#define APIC_VERSION         0x0030
#define APIC_TPR             0x0080
#define APIC_EOI             0x00B0
#define APIC_SVR             0x00F0
#define APIC_ESR             0x0280
#define APIC_ICR_LOW         0x0300
#define APIC_ICR_HIGH        0x0310
#define APIC_LVT_TIMER       0x0320
#define APIC_LVT_THERMAL     0x0330
#define APIC_LVT_PERF        0x0340
#define APIC_LVT_LINT0       0x0350
#define APIC_LVT_LINT1       0x0360
#define APIC_LVT_ERROR       0x0370
#define APIC_TIMER_INIT      0x0380
#define APIC_TIMER_CURRENT   0x0390
#define APIC_TIMER_DIV       0x03E0

// ICR Delivery Mode
#define APIC_ICR_FIXED       0x00000000
#define APIC_ICR_INIT        0x00000500
#define APIC_ICR_STARTUP     0x00000600

// ICR Destination
#define APIC_ICR_DEST_SELF   0x00040000
#define APIC_ICR_DEST_ALL    0x00080000
#define APIC_ICR_DEST_OTHERS 0x000C0000

// Functions
void apic_init(void);
void apic_enable(void);
uint32_t apic_get_id(void);
void apic_eoi(void);
void apic_send_ipi(uint8_t dest_apic_id, uint8_t vector);
void apic_send_init(uint8_t dest_apic_id);
void apic_send_startup(uint8_t dest_apic_id, uint8_t vector);

#endif // APIC_H
