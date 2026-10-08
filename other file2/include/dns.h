#ifndef DNS_H
#define DNS_H

#include <stdint.h>

// DNS constants
#define DNS_PORT 53
#define DNS_MAX_NAME_LEN 255

// DNS header flags
#define DNS_FLAG_QUERY    0x0000
#define DNS_FLAG_RESPONSE 0x8000

// DNS query types
#define DNS_TYPE_A     1   // IPv4 address
#define DNS_TYPE_AAAA  28  // IPv6 address
#define DNS_TYPE_CNAME 5   // Canonical name

// DNS query classes
#define DNS_CLASS_IN 1     // Internet

// DNS header structure
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;  // Question count
    uint16_t ancount;  // Answer count
    uint16_t nscount;  // Authority count
    uint16_t arcount;  // Additional count
} __attribute__((packed)) dns_header_t;

// Functions
void dns_init(void);
int dns_resolve(const char* hostname, uint32_t* ip);
void dns_set_server(uint32_t server_ip);

#endif // DNS_H
