// ============================================================================
// DNS (Domain Name System) Implementation
// Simple DNS resolver for hostname to IP translation
// ============================================================================

#include "../include/dns.h"
#include "../include/udp.h"
#include "../include/socket.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// DNS server address (Google DNS: 8.8.8.8)
static uint32_t dns_server = (8 << 24) | (8 << 16) | (8 << 8) | 8;
static uint16_t dns_query_id = 1;

// ============================================================================
// Utility Functions
// ============================================================================

static uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF) << 8) | ((hostshort >> 8) & 0xFF);
}

static uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 0xFF) << 8) | ((netshort >> 8) & 0xFF);
}

static uint32_t ntohl(uint32_t netlong) {
    return ((netlong & 0xFF) << 24) |
           ((netlong & 0xFF00) << 8) |
           ((netlong >> 8) & 0xFF00) |
           ((netlong >> 24) & 0xFF);
}

static int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// ============================================================================
// Initialization
// ============================================================================

void dns_init(void) {
    terminal_write_line("[DNS] Initializing DNS resolver...");

    terminal_write("[DNS] DNS server: ");
    terminal_write_hex((dns_server >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((dns_server >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((dns_server >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(dns_server & 0xFF);
    terminal_write_line("");

    terminal_write_line("[DNS] Initialization complete");
}

void dns_set_server(uint32_t server_ip) {
    dns_server = server_ip;

    terminal_write("[DNS] DNS server changed to ");
    terminal_write_hex((dns_server >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((dns_server >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((dns_server >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(dns_server & 0xFF);
    terminal_write_line("");
}

// ============================================================================
// Encode DNS Name
// ============================================================================

static int dns_encode_name(const char* hostname, uint8_t* buffer) {
    int len = 0;
    const char* p = hostname;
    uint8_t* label_len = buffer++;
    int label_count = 0;

    while (*p) {
        if (*p == '.') {
            *label_len = label_count;
            label_len = buffer++;
            label_count = 0;
            len++;
            p++;
        } else {
            *buffer++ = *p++;
            label_count++;
            len++;
        }
    }

    *label_len = label_count;
    *buffer = 0;  // Null terminator
    len += 2;     // +1 for last label length, +1 for null

    return len;
}

// ============================================================================
// Resolve Hostname
// ============================================================================

int dns_resolve(const char* hostname, uint32_t* ip) {
    terminal_write("[DNS] Resolving ");
    terminal_write(hostname);
    terminal_write_line("...");

    // Build DNS query
    uint8_t query[512];
    uint8_t* p = query;

    // DNS header
    dns_header_t* header = (dns_header_t*)p;
    header->id = htons(dns_query_id++);
    header->flags = htons(0x0100);  // Standard query
    header->qdcount = htons(1);     // 1 question
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;
    p += sizeof(dns_header_t);

    // Question section
    // Encode hostname
    int name_len = dns_encode_name(hostname, p);
    p += name_len;

    // Query type (A record)
    *p++ = 0;
    *p++ = DNS_TYPE_A;

    // Query class (IN)
    *p++ = 0;
    *p++ = DNS_CLASS_IN;

    uint32_t query_len = p - query;

    // Send DNS query via UDP
    udp_socket_t udp_sock;
    udp_bind(&udp_sock, 53000);  // Use high port

    if (udp_send(&udp_sock, dns_server, DNS_PORT, query, query_len) < 0) {
        terminal_write_line("[DNS] Failed to send query");
        return -1;
    }

    terminal_write_line("[DNS] Query sent, waiting for response...");

    // TODO: Wait for response with timeout
    // TODO: Parse DNS response
    // TODO: Extract IP address from A record

    // For now, return failure
    terminal_write_line("[DNS] Response handling not implemented yet");
    return -1;
}
