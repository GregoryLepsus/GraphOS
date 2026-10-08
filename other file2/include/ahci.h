#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>

// AHCI Device Structure
typedef struct {
    uint8_t port_num;
    uint8_t present;
    uint8_t signature;
    uint64_t capacity_sectors;
    uint32_t sector_size;
    char model[41];
    char serial[21];
    char firmware[9];
} ahci_device_t;

// AHCI Signature Types
#define AHCI_SIG_ATA    0x00000101  // SATA drive
#define AHCI_SIG_ATAPI  0xEB140101  // SATAPI drive
#define AHCI_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define AHCI_SIG_PM     0x96690101  // Port multiplier

// AHCI Port States
#define AHCI_PORT_IDLE    0
#define AHCI_PORT_ACTIVE  1
#define AHCI_PORT_ERROR   2

// Functions
void ahci_init(void);
int ahci_probe_ports(void);
int ahci_read_sectors(uint8_t port, uint64_t lba, uint16_t count, void* buffer);
int ahci_write_sectors(uint8_t port, uint64_t lba, uint16_t count, void* buffer);
int ahci_identify_device(uint8_t port, ahci_device_t* device);
ahci_device_t* ahci_get_device(uint8_t port);
int ahci_get_port_count(void);

#endif // AHCI_H
