#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>

// Ethernet frame constants
#define ETH_ALEN 6              // Ethernet address length
#define ETH_HLEN 14             // Ethernet header length
#define ETH_ZLEN 60             // Minimum frame size
#define ETH_DATA_LEN 1500       // Maximum data length
#define ETH_FRAME_LEN 1514      // Maximum frame length

// EtherTypes
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_ARP  0x0806
#define ETHERTYPE_IPV6 0x86DD

// Ethernet header
typedef struct {
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
} __attribute__((packed)) eth_header_t;

// Ethernet frame
typedef struct {
    eth_header_t header;
    uint8_t payload[ETH_DATA_LEN];
} __attribute__((packed)) eth_frame_t;

// Functions
void ethernet_init(void);
int ethernet_send(uint8_t* dest_mac, uint16_t ethertype, uint8_t* data, uint32_t len);
void ethernet_receive(uint8_t* frame, uint32_t len);
void ethernet_get_mac(uint8_t* mac);

#endif // ETHERNET_H
