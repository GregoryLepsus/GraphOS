#ifndef ARP_H
#define ARP_H

#include <stdint.h>

// ARP constants
#define ARP_HTYPE_ETHERNET 1
#define ARP_PTYPE_IPV4     0x0800

#define ARP_OPER_REQUEST   1
#define ARP_OPER_REPLY     2

#define ARP_CACHE_SIZE     32
#define ARP_CACHE_TIMEOUT  300  // 5 minutes in seconds

// ARP header
typedef struct {
    uint16_t htype;         // Hardware type
    uint16_t ptype;         // Protocol type
    uint8_t hlen;           // Hardware address length
    uint8_t plen;           // Protocol address length
    uint16_t oper;          // Operation
    uint8_t sha[6];         // Sender hardware address
    uint32_t spa;           // Sender protocol address
    uint8_t tha[6];         // Target hardware address
    uint32_t tpa;           // Target protocol address
} __attribute__((packed)) arp_header_t;

// ARP cache entry
typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    uint32_t timestamp;
    int valid;
} arp_cache_entry_t;

// Functions
void arp_init(void);
void arp_receive(uint8_t* packet, uint32_t len);
int arp_resolve(uint32_t ip, uint8_t* mac);
void arp_send_request(uint32_t target_ip);
void arp_add_entry(uint32_t ip, uint8_t* mac);
void arp_print_cache(void);

#endif // ARP_H
