// ============================================================================
// Ethernet Layer Implementation
// Link layer protocol handling
// ============================================================================

#include "../include/ethernet.h"
#include "../include/e1000.h"
#include "../include/arp.h"
#include "../include/ip.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Local MAC address
static uint8_t local_mac[6];

// ============================================================================
// Utility Functions
// ============================================================================

static uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF) << 8) | ((hostshort >> 8) & 0xFF);
}

static uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 0xFF) << 8) | ((netshort >> 8) & 0xFF);
}

static void mac_copy(uint8_t* dest, const uint8_t* src) {
    for (int i = 0; i < 6; i++) {
        dest[i] = src[i];
    }
}

static int mac_equals(const uint8_t* mac1, const uint8_t* mac2) {
    for (int i = 0; i < 6; i++) {
        if (mac1[i] != mac2[i]) return 0;
    }
    return 1;
}

static int mac_is_broadcast(const uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0xFF) return 0;
    }
    return 1;
}

// ============================================================================
// Initialization
// ============================================================================

void ethernet_init(void) {
    terminal_write_line("[Ethernet] Initializing Ethernet layer...");

    // Get MAC address from network driver
    e1000_get_mac_addr(local_mac);

    terminal_write("[Ethernet] MAC Address: ");
    for (int i = 0; i < 6; i++) {
        terminal_write_hex(local_mac[i]);
        if (i < 5) terminal_write(":");
    }
    terminal_write_line("");

    terminal_write_line("[Ethernet] Initialization complete");
}

void ethernet_get_mac(uint8_t* mac) {
    mac_copy(mac, local_mac);
}

// ============================================================================
// Send Frame
// ============================================================================

int ethernet_send(uint8_t* dest_mac, uint16_t ethertype, uint8_t* data, uint32_t len) {
    if (len > ETH_DATA_LEN) {
        return -1;  // Payload too large
    }

    // Build Ethernet frame
    uint8_t frame[ETH_FRAME_LEN];
    eth_header_t* header = (eth_header_t*)frame;

    // Fill header
    mac_copy(header->dest_mac, dest_mac);
    mac_copy(header->src_mac, local_mac);
    header->ethertype = htons(ethertype);

    // Copy payload
    uint8_t* payload = frame + ETH_HLEN;
    for (uint32_t i = 0; i < len; i++) {
        payload[i] = data[i];
    }

    // Pad to minimum frame size if needed
    uint32_t frame_len = ETH_HLEN + len;
    if (frame_len < ETH_ZLEN) {
        // Zero-pad
        for (uint32_t i = frame_len; i < ETH_ZLEN; i++) {
            frame[i] = 0;
        }
        frame_len = ETH_ZLEN;
    }

    // Send via network driver
    return e1000_send(frame, frame_len);
}

// ============================================================================
// Receive Frame
// ============================================================================

void ethernet_receive(uint8_t* frame, uint32_t len) {
    if (len < ETH_HLEN) {
        return;  // Frame too small
    }

    eth_header_t* header = (eth_header_t*)frame;

    // Check destination MAC
    if (!mac_equals(header->dest_mac, local_mac) &&
        !mac_is_broadcast(header->dest_mac)) {
        return;  // Not for us
    }

    // Extract payload
    uint8_t* payload = frame + ETH_HLEN;
    uint32_t payload_len = len - ETH_HLEN;

    // Get ethertype
    uint16_t ethertype = ntohs(header->ethertype);

    // Dispatch to protocol handler
    switch (ethertype) {
        case ETHERTYPE_ARP:
            arp_receive(payload, payload_len);
            break;

        case ETHERTYPE_IPV4:
            ip_receive(payload, payload_len);
            break;

        case ETHERTYPE_IPV6:
            terminal_write_line("[Ethernet] Received IPv6 packet (not supported)");
            break;

        default:
            terminal_write("[Ethernet] Unknown ethertype: ");
            terminal_write_hex(ethertype);
            terminal_write_line("");
            break;
    }
}
