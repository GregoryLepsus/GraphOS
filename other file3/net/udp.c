// ============================================================================
// UDP (User Datagram Protocol) Implementation
// Connectionless transport protocol
// ============================================================================

#include "../include/udp.h"
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

void udp_init(void) {
    terminal_write_line("[UDP] Initializing UDP protocol...");
    terminal_write_line("[UDP] Initialization complete");
}

// ============================================================================
// Bind UDP Socket
// ============================================================================

int udp_bind(udp_socket_t* sock, uint16_t port) {
    if (!sock) return -1;

    sock->local_port = port;
    sock->remote_ip = 0;
    sock->remote_port = 0;
    sock->bound = 1;

    terminal_write("[UDP] Bound to port ");
    terminal_write_hex(port);
    terminal_write_line("");

    return 0;
}

// ============================================================================
// Send UDP Datagram
// ============================================================================

int udp_send(udp_socket_t* sock, uint32_t dest_ip, uint16_t dest_port, uint8_t* data, uint32_t len) {
    if (!sock || !sock->bound) return -1;

    // Build UDP header
    udp_header_t header;
    header.src_port = htons(sock->local_port);
    header.dest_port = htons(dest_port);
    header.length = htons(sizeof(udp_header_t) + len);
    header.checksum = 0;  // Optional for IPv4

    // Build complete packet
    uint32_t packet_len = sizeof(udp_header_t) + len;
    uint8_t packet[packet_len];

    // Copy header
    uint8_t* p = packet;
    for (uint32_t i = 0; i < sizeof(udp_header_t); i++) {
        p[i] = ((uint8_t*)&header)[i];
    }

    // Copy data
    p += sizeof(udp_header_t);
    for (uint32_t i = 0; i < len; i++) {
        p[i] = data[i];
    }

    // Send via IP layer
    return ip_send(dest_ip, IP_PROTO_UDP, packet, packet_len);
}

// ============================================================================
// Receive UDP Packet
// ============================================================================

void udp_receive_packet(uint32_t src_ip, uint8_t* packet, uint32_t len) {
    if (len < sizeof(udp_header_t)) {
        return;  // Packet too small
    }

    udp_header_t* header = (udp_header_t*)packet;

    uint16_t src_port = ntohs(header->src_port);
    uint16_t dest_port = ntohs(header->dest_port);
    uint16_t length = ntohs(header->length);

    terminal_write("[UDP] Packet from ");
    terminal_write_hex((src_ip >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((src_ip >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((src_ip >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(src_ip & 0xFF);
    terminal_write(":");
    terminal_write_hex(src_port);
    terminal_write(" to port ");
    terminal_write_hex(dest_port);
    terminal_write_line("");

    // TODO: Deliver to socket
}

// ============================================================================
// Receive from UDP Socket
// ============================================================================

int udp_receive(udp_socket_t* sock, uint8_t* buf, uint32_t max_len, uint32_t* src_ip, uint16_t* src_port) {
    // TODO: Implement socket receive queue
    return 0;
}
