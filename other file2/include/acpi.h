#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

// ACPI RSDP Structure
typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed)) acpi_rsdp_t;

// ACPI Table Header
typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_table_header_t;

// ACPI System State
typedef enum {
    ACPI_STATE_S0,  // Working
    ACPI_STATE_S1,  // Sleep (CPU off, RAM on)
    ACPI_STATE_S2,  // Sleep (CPU off, RAM slow)
    ACPI_STATE_S3,  // Sleep (Suspend to RAM)
    ACPI_STATE_S4,  // Sleep (Suspend to disk)
    ACPI_STATE_S5   // Soft off
} acpi_state_t;

// ACPI Info Structure
typedef struct {
    uint64_t rsdp_address;
    uint32_t rsdt_address;
    uint32_t fadt_address;
    uint8_t acpi_version;
    uint8_t century_register;
    uint16_t pm1a_control_block;
    uint16_t pm1b_control_block;
    uint16_t slp_typa;
    uint16_t slp_typb;
} acpi_info_t;

// Functions
void acpi_init(void);
int acpi_find_rsdp(void);
int acpi_parse_tables(void);
void acpi_get_info(acpi_info_t* info);
int acpi_shutdown(void);
int acpi_reboot(void);
int acpi_sleep(acpi_state_t state);
int acpi_enable(void);

#endif // ACPI_H
