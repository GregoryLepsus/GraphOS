// ============================================================================
// ARP (Address Resolution Protocol) Implementation
// Maps IP addresses to MAC addresses
// ============================================================================

#include "../include/arp.h"
#include "../include/ethernet.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);
extern uint32_t timer_get_ticks(void);

// ARP cache
static arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];
static uint32_t local_ip = 0;

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

static void mac_copy(uint8_t* dest, const uint8_t* src) {
    for (int i = 0; i < 6; i++) {
        dest[i] = src[i];
    }
}

// ============================================================================
// Initialization
// ============================================================================

void arp_init(void) {
    terminal_write_line("[ARP] Initializing ARP protocol...");

    // Clear ARP cache
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        arp_cache[i].valid = 0;
        arp_cache[i].ip = 0;
        arp_cache[i].timestamp = 0;
    }

    // Set local IP (example: 192.168.1.100)
    local_ip = (192 << 24) | (168 << 16) | (1 << 8) | 100;

    terminal_write("[ARP] Local IP: ");
    terminal_write_hex((local_ip >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((local_ip >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((local_ip >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(local_ip & 0xFF);
    terminal_write_line("");

    terminal_write_line("[ARP] Initialization complete");
}

// ============================================================================
// ARP Cache Management
// ============================================================================

void arp_add_entry(uint32_t ip, uint8_t* mac) {
    uint32_t now = timer_get_ticks();

    // Check if entry already exists
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            // Update existing entry
            mac_copy(arp_cache[i].mac, mac);
            arp_cache[i].timestamp = now;
            return;
        }
    }

    // Find free slot
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (!arp_cache[i].valid) {
            arp_cache[i].ip = ip;
            mac_copy(arp_cache[i].mac, mac);
            arp_cache[i].timestamp = now;
            arp_cache[i].valid = 1;
            return;
        }
    }

    // Cache full - replace oldest entry
    int oldest = 0;
    for (int i = 1; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].timestamp < arp_cache[oldest].timestamp) {
            oldest = i;
        }
    }

    arp_cache[oldest].ip = ip;
    mac_copy(arp_cache[oldest].mac, mac);
    arp_cache[oldest].timestamp = now;
    arp_cache[oldest].valid = 1;
}

int arp_resolve(uint32_t ip, uint8_t* mac) {
    uint32_t now = timer_get_ticks();

    // Search cache
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip) {
            // Check if entry is still valid
            if ((now - arp_cache[i].timestamp) < ARP_CACHE_TIMEOUT * 1000) {
                mac_copy(mac, arp_cache[i].mac);
                return 1;  // Found
            } else {
                // Entry expired
                arp_cache[i].valid = 0;
            }
        }
    }

    return 0;  // Not found
}

// ============================================================================
// Send ARP Request
// ============================================================================

void arp_send_request(uint32_t target_ip) {
    arp_header_t arp;
    uint8_t local_mac[6];

    // Get local MAC address
    ethernet_get_mac(local_mac);

    // Build ARP request
    arp.htype = htons(ARP_HTYPE_ETHERNET);
    arp.ptype = htons(ARP_PTYPE_IPV4);
    arp.hlen = 6;
    arp.plen = 4;
    arp.oper = htons(ARP_OPER_REQUEST);

    mac_copy(arp.sha, local_mac);
    arp.spa = htonl(local_ip);

    // Target hardware address is unknown (set to 0)
    for (int i = 0; i < 6; i++) {
        arp.tha[i] = 0;
    }
    arp.tpa = htonl(target_ip);

    // Send as Ethernet broadcast
    uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ethernet_send(broadcast_mac, ETHERTYPE_ARP, (uint8_t*)&arp, sizeof(arp));

    terminal_write("[ARP] Sent ARP request for ");
    terminal_write_hex((target_ip >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((target_ip >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((target_ip >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(target_ip & 0xFF);
    terminal_write_line("");
}

// ============================================================================
// Receive ARP Packet
// ============================================================================

void arp_receive(uint8_t* packet, uint32_t len) {
    if (len < sizeof(arp_header_t)) {
        return;  // Packet too small
    }

    arp_header_t* arp = (arp_header_t*)packet;

    // Check hardware and protocol types
    if (ntohs(arp->htype) != ARP_HTYPE_ETHERNET ||
        ntohs(arp->ptype) != ARP_PTYPE_IPV4) {
        return;
    }

    uint32_t sender_ip = ntohl(arp->spa);
    uint32_t target_ip = ntohl(arp->tpa);
    uint16_t operation = ntohs(arp->oper);

    // Add sender to cache
    arp_add_entry(sender_ip, arp->sha);

    // Check if request is for us
    if (target_ip != local_ip) {
        return;
    }

    if (operation == ARP_OPER_REQUEST) {
        // Send ARP reply
        arp_header_t reply;
        uint8_t local_mac[6];

        ethernet_get_mac(local_mac);

        reply.htype = htons(ARP_HTYPE_ETHERNET);
        reply.ptype = htons(ARP_PTYPE_IPV4);
        reply.hlen = 6;
        reply.plen = 4;
        reply.oper = htons(ARP_OPER_REPLY);

        mac_copy(reply.sha, local_mac);
        reply.spa = htonl(local_ip);
        mac_copy(reply.tha, arp->sha);
        reply.tpa = arp->spa;

        ethernet_send(arp->sha, ETHERTYPE_ARP, (uint8_t*)&reply, sizeof(reply));

        terminal_write_line("[ARP] Sent ARP reply");
    } else if (operation == ARP_OPER_REPLY) {
        terminal_write_line("[ARP] Received ARP reply");
    }
}

// ============================================================================
// Print ARP Cache
// ============================================================================

void arp_print_cache(void) {
    terminal_write_line("ARP Cache:");

    int count = 0;
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid) {
            uint32_t ip = arp_cache[i].ip;

            terminal_write("  ");
            terminal_write_hex((ip >> 24) & 0xFF);
            terminal_write(".");
            terminal_write_hex((ip >> 16) & 0xFF);
            terminal_write(".");
            terminal_write_hex((ip >> 8) & 0xFF);
            terminal_write(".");
            terminal_write_hex(ip & 0xFF);
            terminal_write(" -> ");

            for (int j = 0; j < 6; j++) {
                terminal_write_hex(arp_cache[i].mac[j]);
                if (j < 5) terminal_write(":");
            }
            terminal_write_line("");

            count++;
        }
    }

    if (count == 0) {
        terminal_write_line("  (empty)");
    }
}
