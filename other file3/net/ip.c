// ============================================================================
// IP (Internet Protocol) Layer Implementation
// Network layer packet routing
// ============================================================================

#include "../include/ip.h"
#include "../include/ethernet.h"
#include "../include/arp.h"
#include "../include/icmp.h"
#include "../include/tcp.h"
#include "../include/udp.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Network configuration
static uint32_t local_ip = 0;
static uint32_t netmask = 0;
static uint32_t gateway = 0;
static uint16_t ip_id = 0;

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
// IP Checksum
// ============================================================================

uint16_t ip_checksum(uint16_t* data, int len) {
    uint32_t sum = 0;

    while (len > 1) {
        sum += *data++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(uint8_t*)data;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

// ============================================================================
// Initialization
// ============================================================================

void ip_init(void) {
    terminal_write_line("[IP] Initializing IP layer...");

    // Default configuration: 192.168.1.100/24, gateway 192.168.1.1
    local_ip = (192 << 24) | (168 << 16) | (1 << 8) | 100;
    netmask = (255 << 24) | (255 << 16) | (255 << 8) | 0;
    gateway = (192 << 24) | (168 << 16) | (1 << 8) | 1;

    terminal_write("[IP] Local IP: ");
    terminal_write_hex((local_ip >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((local_ip >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((local_ip >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(local_ip & 0xFF);
    terminal_write_line("");

    terminal_write_line("[IP] Initialization complete");
}

void ip_set_address(uint32_t ip, uint32_t nm, uint32_t gw) {
    local_ip = ip;
    netmask = nm;
    gateway = gw;

    terminal_write("[IP] Address updated: ");
    terminal_write_hex((ip >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((ip >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((ip >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(ip & 0xFF);
    terminal_write_line("");
}

// ============================================================================
// Send IP Packet
// ============================================================================

int ip_send(uint32_t dest_ip, uint8_t protocol, uint8_t* data, uint32_t len) {
    // Build IP header
    ip_header_t header;
    header.version_ihl = (IP_VERSION << 4) | IP_HLEN;
    header.tos = 0;
    header.total_length = htons(sizeof(ip_header_t) + len);
    header.identification = htons(ip_id++);
    header.flags_offset = htons(IP_FLAG_DONT_FRAGMENT);
    header.ttl = 64;
    header.protocol = protocol;
    header.checksum = 0;
    header.src_ip = htonl(local_ip);
    header.dest_ip = htonl(dest_ip);

    // Calculate checksum
    header.checksum = ip_checksum((uint16_t*)&header, sizeof(ip_header_t));

    // Build complete packet
    uint32_t packet_len = sizeof(ip_header_t) + len;
    uint8_t packet[packet_len];

    // Copy header
    uint8_t* p = packet;
    for (uint32_t i = 0; i < sizeof(ip_header_t); i++) {
        p[i] = ((uint8_t*)&header)[i];
    }

    // Copy data
    p += sizeof(ip_header_t);
    for (uint32_t i = 0; i < len; i++) {
        p[i] = data[i];
    }

    // Resolve MAC address
    uint8_t dest_mac[6];
    uint32_t next_hop = dest_ip;

    // Check if destination is on local network
    if ((dest_ip & netmask) != (local_ip & netmask)) {
        // Use gateway
        next_hop = gateway;
    }

    if (!arp_resolve(next_hop, dest_mac)) {
        // MAC not in cache, send ARP request
        arp_send_request(next_hop);
        return -1;  // Packet dropped (should queue in real implementation)
    }

    // Send via Ethernet
    return ethernet_send(dest_mac, ETHERTYPE_IPV4, packet, packet_len);
}

// ============================================================================
// Receive IP Packet
// ============================================================================

void ip_receive(uint8_t* packet, uint32_t len) {
    if (len < sizeof(ip_header_t)) {
        return;  // Packet too small
    }

    ip_header_t* header = (ip_header_t*)packet;

    // Check version
    uint8_t version = header->version_ihl >> 4;
    if (version != IP_VERSION) {
        return;
    }

    // Check destination
    uint32_t dest = ntohl(header->dest_ip);
    if (dest != local_ip && dest != 0xFFFFFFFF) {
        return;  // Not for us
    }

    // Verify checksum
    uint16_t orig_checksum = header->checksum;
    header->checksum = 0;
    uint16_t calc_checksum = ip_checksum((uint16_t*)header, sizeof(ip_header_t));

    if (orig_checksum != calc_checksum) {
        terminal_write_line("[IP] Checksum mismatch");
        return;
    }

    // Extract payload
    uint8_t ihl = header->version_ihl & 0x0F;
    uint32_t header_len = ihl * 4;
    uint8_t* payload = packet + header_len;
    uint32_t payload_len = len - header_len;

    uint32_t src_ip = ntohl(header->src_ip);

    // Dispatch to protocol handler
    switch (header->protocol) {
        case IP_PROTO_ICMP:
            icmp_receive(src_ip, payload, payload_len);
            break;

        case IP_PROTO_TCP:
            tcp_receive_packet(src_ip, payload, payload_len);
            break;

        case IP_PROTO_UDP:
            udp_receive_packet(src_ip, payload, payload_len);
            break;

        default:
            terminal_write("[IP] Unknown protocol: ");
            terminal_write_hex(header->protocol);
            terminal_write_line("");
            break;
    }
}
