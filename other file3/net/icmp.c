// ============================================================================
// ICMP (Internet Control Message Protocol) Implementation
// Network diagnostic and control messages (ping)
// ============================================================================

#include "../include/icmp.h"
#include "../include/ip.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// ============================================================================
// Utility Functions
// ============================================================================

static uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF) << 8) | ((hostshort >> 8) & 0xFF);
}

static uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 0xFF) << 8) | ((netshort >> 8) & 0xFF);
}

// ============================================================================
// Initialization
// ============================================================================

void icmp_init(void) {
    terminal_write_line("[ICMP] Initializing ICMP protocol...");
    terminal_write_line("[ICMP] Initialization complete");
}

// ============================================================================
// Send ICMP Echo Request (Ping)
// ============================================================================

int icmp_send_echo_request(uint32_t dest_ip, uint16_t id, uint16_t seq) {
    // Build ICMP echo request
    icmp_header_t header;
    header.type = ICMP_TYPE_ECHO_REQUEST;
    header.code = ICMP_CODE_ECHO;
    header.checksum = 0;
    header.identifier = htons(id);
    header.sequence = htons(seq);

    // Add some payload
    uint8_t payload[32];
    for (int i = 0; i < 32; i++) {
        payload[i] = 0x41 + (i % 26);  // 'A' to 'Z'
    }

    // Build complete packet
    uint32_t packet_len = sizeof(icmp_header_t) + 32;
    uint8_t packet[packet_len];

    // Copy header
    uint8_t* p = packet;
    for (uint32_t i = 0; i < sizeof(icmp_header_t); i++) {
        p[i] = ((uint8_t*)&header)[i];
    }

    // Copy payload
    p += sizeof(icmp_header_t);
    for (int i = 0; i < 32; i++) {
        p[i] = payload[i];
    }

    // Calculate checksum
    icmp_header_t* hdr = (icmp_header_t*)packet;
    hdr->checksum = ip_checksum((uint16_t*)packet, packet_len);

    // Send via IP layer
    return ip_send(dest_ip, IP_PROTO_ICMP, packet, packet_len);
}

// ============================================================================
// Receive ICMP Packet
// ============================================================================

void icmp_receive(uint32_t src_ip, uint8_t* packet, uint32_t len) {
    if (len < sizeof(icmp_header_t)) {
        return;  // Packet too small
    }

    icmp_header_t* header = (icmp_header_t*)packet;

    switch (header->type) {
        case ICMP_TYPE_ECHO_REQUEST: {
            // Send echo reply
            terminal_write("[ICMP] Echo request from ");
            terminal_write_hex((src_ip >> 24) & 0xFF);
            terminal_write(".");
            terminal_write_hex((src_ip >> 16) & 0xFF);
            terminal_write(".");
            terminal_write_hex((src_ip >> 8) & 0xFF);
            terminal_write(".");
            terminal_write_hex(src_ip & 0xFF);
            terminal_write_line("");

            // Build reply
            uint8_t reply[len];
            for (uint32_t i = 0; i < len; i++) {
                reply[i] = packet[i];
            }

            icmp_header_t* reply_hdr = (icmp_header_t*)reply;
            reply_hdr->type = ICMP_TYPE_ECHO_REPLY;
            reply_hdr->checksum = 0;
            reply_hdr->checksum = ip_checksum((uint16_t*)reply, len);

            // Send reply
            ip_send(src_ip, IP_PROTO_ICMP, reply, len);
            terminal_write_line("[ICMP] Sent echo reply");
            break;
        }

        case ICMP_TYPE_ECHO_REPLY:
            terminal_write("[ICMP] Echo reply from ");
            terminal_write_hex((src_ip >> 24) & 0xFF);
            terminal_write(".");
            terminal_write_hex((src_ip >> 16) & 0xFF);
            terminal_write(".");
            terminal_write_hex((src_ip >> 8) & 0xFF);
            terminal_write(".");
            terminal_write_hex(src_ip & 0xFF);
            terminal_write(" seq=");
            terminal_write_hex(ntohs(header->sequence));
            terminal_write_line("");
            break;

        default:
            terminal_write("[ICMP] Unknown type: ");
            terminal_write_hex(header->type);
            terminal_write_line("");
            break;
    }
}
