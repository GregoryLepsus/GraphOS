// ============================================================================
// TCP (Transmission Control Protocol) Implementation
// Connection-oriented reliable transport protocol
// ============================================================================

#include "../include/tcp.h"
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

static uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF) << 24) |
           ((hostlong & 0xFF00) << 8) |
           ((hostlong >> 8) & 0xFF00) |
           ((hostlong >> 24) & 0xFF);
}

static uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong);
}

// ============================================================================
// Initialization
// ============================================================================

void tcp_init(void) {
    terminal_write_line("[TCP] Initializing TCP protocol...");
    terminal_write_line("[TCP] Initialization complete");
}

// ============================================================================
// Send TCP Segment
// ============================================================================

static int tcp_send_segment(tcp_socket_t* sock, uint8_t* data, uint32_t len, uint16_t flags) {
    // Build TCP header
    tcp_header_t header;
    header.src_port = htons(sock->local_port);
    header.dest_port = htons(sock->remote_port);
    header.seq_num = htonl(sock->seq_num);
    header.ack_num = htonl(sock->ack_num);
    header.flags = htons((5 << 12) | flags);  // Data offset = 5 (20 bytes)
    header.window = htons(sock->window);
    header.checksum = 0;  // TODO: Calculate TCP checksum
    header.urgent_ptr = 0;

    // Build complete packet
    uint32_t packet_len = sizeof(tcp_header_t) + len;
    uint8_t packet[packet_len];

    // Copy header
    uint8_t* p = packet;
    for (uint32_t i = 0; i < sizeof(tcp_header_t); i++) {
        p[i] = ((uint8_t*)&header)[i];
    }

    // Copy data
    if (data && len > 0) {
        p += sizeof(tcp_header_t);
        for (uint32_t i = 0; i < len; i++) {
            p[i] = data[i];
        }
    }

    // Send via IP layer
    return ip_send(sock->remote_ip, IP_PROTO_TCP, packet, packet_len);
}

// ============================================================================
// TCP Connect (3-way handshake)
// ============================================================================

int tcp_connect(tcp_socket_t* sock, uint32_t dest_ip, uint16_t dest_port) {
    if (!sock) return -1;

    // Initialize socket
    sock->remote_ip = dest_ip;
    sock->remote_port = dest_port;
    sock->local_port = 12345;  // TODO: Allocate dynamic port
    sock->seq_num = 1000;      // TODO: Random initial sequence number
    sock->ack_num = 0;
    sock->window = 8192;
    sock->state = TCP_SYN_SENT;

    // Send SYN
    terminal_write_line("[TCP] Sending SYN...");
    tcp_send_segment(sock, NULL, 0, TCP_FLAG_SYN);

    // TODO: Wait for SYN-ACK and send ACK

    return 0;
}

// ============================================================================
// TCP Send Data
// ============================================================================

int tcp_send(tcp_socket_t* sock, uint8_t* data, uint32_t len) {
    if (!sock || sock->state != TCP_ESTABLISHED) {
        return -1;
    }

    // Send data with PSH and ACK flags
    tcp_send_segment(sock, data, len, TCP_FLAG_PSH | TCP_FLAG_ACK);

    // Update sequence number
    sock->seq_num += len;

    return len;
}

// ============================================================================
// TCP Close Connection
// ============================================================================

void tcp_close(tcp_socket_t* sock) {
    if (!sock) return;

    if (sock->state == TCP_ESTABLISHED) {
        // Send FIN
        terminal_write_line("[TCP] Sending FIN...");
        tcp_send_segment(sock, NULL, 0, TCP_FLAG_FIN | TCP_FLAG_ACK);
        sock->state = TCP_FIN_WAIT_1;
    }

    // TODO: Wait for FIN-ACK
}

// ============================================================================
// Receive TCP Packet
// ============================================================================

void tcp_receive_packet(uint32_t src_ip, uint8_t* packet, uint32_t len) {
    if (len < sizeof(tcp_header_t)) {
        return;  // Packet too small
    }

    tcp_header_t* header = (tcp_header_t*)packet;

    uint16_t src_port = ntohs(header->src_port);
    uint16_t dest_port = ntohs(header->dest_port);
    uint32_t seq_num = ntohl(header->seq_num);
    uint32_t ack_num = ntohl(header->ack_num);
    uint16_t flags = ntohs(header->flags) & 0x3F;

    terminal_write("[TCP] Packet from ");
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
    terminal_write(" flags=");
    terminal_write_hex(flags);
    terminal_write_line("");

    // TODO: Process based on state machine
    // TODO: Handle SYN, ACK, FIN, RST, etc.
}

// ============================================================================
// TCP Receive Data
// ============================================================================

int tcp_receive(tcp_socket_t* sock, uint8_t* buf, uint32_t max_len) {
    // TODO: Implement receive buffer
    return 0;
}
