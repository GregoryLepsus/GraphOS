#ifndef E1000_H
#define E1000_H

#include <stdint.h>

// Intel E1000 Vendor/Device IDs
#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100E  // 82540EM

// E1000 Register Offsets
#define E1000_REG_CTRL     0x0000  // Device Control
#define E1000_REG_STATUS   0x0008  // Device Status
#define E1000_REG_EECD     0x0010  // EEPROM Control
#define E1000_REG_EERD     0x0014  // EEPROM Read
#define E1000_REG_ICR      0x00C0  // Interrupt Cause Read
#define E1000_REG_IMS      0x00D0  // Interrupt Mask Set
#define E1000_REG_IMC      0x00D8  // Interrupt Mask Clear
#define E1000_REG_RCTL     0x0100  // Receive Control
#define E1000_REG_TCTL     0x0400  // Transmit Control
#define E1000_REG_RDBAL    0x2800  // RX Descriptor Base Low
#define E1000_REG_RDBAH    0x2804  // RX Descriptor Base High
#define E1000_REG_RDLEN    0x2808  // RX Descriptor Length
#define E1000_REG_RDH      0x2810  // RX Descriptor Head
#define E1000_REG_RDT      0x2818  // RX Descriptor Tail
#define E1000_REG_TDBAL    0x3800  // TX Descriptor Base Low
#define E1000_REG_TDBAH    0x3804  // TX Descriptor Base High
#define E1000_REG_TDLEN    0x3808  // TX Descriptor Length
#define E1000_REG_TDH      0x3810  // TX Descriptor Head
#define E1000_REG_TDT      0x3818  // TX Descriptor Tail
#define E1000_REG_RAL      0x5400  // Receive Address Low
#define E1000_REG_RAH      0x5404  // Receive Address High

// Control Register Bits
#define E1000_CTRL_FD       0x00000001  // Full Duplex
#define E1000_CTRL_ASDE     0x00000020  // Auto Speed Detection
#define E1000_CTRL_SLU      0x00000040  // Set Link Up
#define E1000_CTRL_RST      0x04000000  // Device Reset

// Receive Control Register Bits
#define E1000_RCTL_EN       0x00000002  // Enable
#define E1000_RCTL_UPE      0x00000008  // Unicast Promiscuous
#define E1000_RCTL_MPE      0x00000010  // Multicast Promiscuous
#define E1000_RCTL_BAM      0x00008000  // Broadcast Accept Mode
#define E1000_RCTL_BSIZE_2K 0x00000000  // Buffer Size 2048

// Transmit Control Register Bits
#define E1000_TCTL_EN       0x00000002  // Enable
#define E1000_TCTL_PSP      0x00000008  // Pad Short Packets

// Descriptor Status Bits
#define E1000_DESC_STATUS_DD  0x01  // Descriptor Done
#define E1000_DESC_STATUS_EOP 0x02  // End of Packet

// Ring Buffer Sizes
#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 32

// Receive Descriptor
typedef struct {
    uint64_t buffer_addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed)) e1000_rx_desc_t;

// Transmit Descriptor
typedef struct {
    uint64_t buffer_addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed)) e1000_tx_desc_t;

// E1000 Device Structure
typedef struct {
    uint32_t mmio_base;
    uint8_t mac_addr[6];
    uint8_t irq;

    // RX Ring
    e1000_rx_desc_t* rx_descs;
    uint32_t rx_descs_phys;
    uint8_t** rx_buffers;
    int rx_tail;

    // TX Ring
    e1000_tx_desc_t* tx_descs;
    uint32_t tx_descs_phys;
    uint8_t** tx_buffers;
    int tx_tail;

    int link_up;
} e1000_device_t;

// Functions
int e1000_init(void);
int e1000_send(uint8_t* data, uint32_t len);
int e1000_receive(uint8_t* buf, uint32_t max_len);
void e1000_get_mac_addr(uint8_t* mac);

#endif // E1000_H
