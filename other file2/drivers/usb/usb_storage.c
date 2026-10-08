// ============================================================================
// USB Mass Storage Driver
// Support for USB flash drives and external hard drives
// ============================================================================

#include "../../include/usb.h"
#include "../../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Mass Storage Class-Specific Requests
#define MSC_GET_MAX_LUN     0xFE
#define MSC_BULK_ONLY_RESET 0xFF

// Command Block Wrapper (CBW)
typedef struct {
    uint32_t signature;      // 0x43425355 "USBC"
    uint32_t tag;
    uint32_t data_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t cb_length;
    uint8_t cb[16];
} __attribute__((packed)) usb_msc_cbw_t;

// Command Status Wrapper (CSW)
typedef struct {
    uint32_t signature;      // 0x53425355 "USBS"
    uint32_t tag;
    uint32_t data_residue;
    uint8_t status;
} __attribute__((packed)) usb_msc_csw_t;

// SCSI Commands
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_REQUEST_SENSE   0x03
#define SCSI_INQUIRY         0x12
#define SCSI_READ_CAPACITY   0x25
#define SCSI_READ_10         0x28
#define SCSI_WRITE_10        0x2A

static uint32_t next_tag = 1;

// ============================================================================
// Mass Storage Initialization
// ============================================================================

int usb_msc_init(usb_device_t* dev) {
    if (!dev) {
        return -1;
    }

    terminal_write_line("[USB MSC] Initializing mass storage device...");

    // Get max LUN
    uint8_t max_lun;
    int result = usb_control_transfer(dev, 0xA1, MSC_GET_MAX_LUN,
                                     0, 0, &max_lun, 1);
    if (result < 0) {
        max_lun = 0;
    }

    terminal_write("[USB MSC] Max LUN: ");
    terminal_write_hex(max_lun);
    terminal_write_line("");

    terminal_write_line("[USB MSC] Mass storage initialized");
    return 0;
}

// ============================================================================
// Read Sectors
// ============================================================================

int usb_msc_read_sectors(usb_device_t* dev, uint32_t lba, uint16_t count, void* buffer) {
    if (!dev || !buffer) {
        return -1;
    }

    // Build CBW
    usb_msc_cbw_t cbw;
    cbw.signature = 0x43425355;
    cbw.tag = next_tag++;
    cbw.data_length = count * 512;
    cbw.flags = 0x80;  // IN
    cbw.lun = 0;
    cbw.cb_length = 10;

    // SCSI READ(10) command
    cbw.cb[0] = SCSI_READ_10;
    cbw.cb[1] = 0;
    cbw.cb[2] = (lba >> 24) & 0xFF;
    cbw.cb[3] = (lba >> 16) & 0xFF;
    cbw.cb[4] = (lba >> 8) & 0xFF;
    cbw.cb[5] = lba & 0xFF;
    cbw.cb[6] = 0;
    cbw.cb[7] = (count >> 8) & 0xFF;
    cbw.cb[8] = count & 0xFF;
    cbw.cb[9] = 0;

    // Send CBW
    usb_bulk_transfer(dev, 0x01, &cbw, sizeof(cbw), 0);

    // Receive data
    usb_bulk_transfer(dev, 0x82, buffer, cbw.data_length, 1);

    // Receive CSW
    usb_msc_csw_t csw;
    usb_bulk_transfer(dev, 0x82, &csw, sizeof(csw), 1);

    return (csw.status == 0) ? 0 : -1;
}

// ============================================================================
// Write Sectors
// ============================================================================

int usb_msc_write_sectors(usb_device_t* dev, uint32_t lba, uint16_t count, void* buffer) {
    if (!dev || !buffer) {
        return -1;
    }

    // Build CBW
    usb_msc_cbw_t cbw;
    cbw.signature = 0x43425355;
    cbw.tag = next_tag++;
    cbw.data_length = count * 512;
    cbw.flags = 0x00;  // OUT
    cbw.lun = 0;
    cbw.cb_length = 10;

    // SCSI WRITE(10) command
    cbw.cb[0] = SCSI_WRITE_10;
    cbw.cb[1] = 0;
    cbw.cb[2] = (lba >> 24) & 0xFF;
    cbw.cb[3] = (lba >> 16) & 0xFF;
    cbw.cb[4] = (lba >> 8) & 0xFF;
    cbw.cb[5] = lba & 0xFF;
    cbw.cb[6] = 0;
    cbw.cb[7] = (count >> 8) & 0xFF;
    cbw.cb[8] = count & 0xFF;
    cbw.cb[9] = 0;

    // Send CBW
    usb_bulk_transfer(dev, 0x01, &cbw, sizeof(cbw), 0);

    // Send data
    usb_bulk_transfer(dev, 0x01, buffer, cbw.data_length, 0);

    // Receive CSW
    usb_msc_csw_t csw;
    usb_bulk_transfer(dev, 0x82, &csw, sizeof(csw), 1);

    return (csw.status == 0) ? 0 : -1;
}
