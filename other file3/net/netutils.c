// ============================================================================
// Network Utilities
// Helper functions for network operations
// ============================================================================

#include "../include/netutils.h"

// ============================================================================
// Network Byte Order Conversion
// ============================================================================

uint16_t htons(uint16_t hostshort) {
    return ((hostshort & 0xFF) << 8) | ((hostshort >> 8) & 0xFF);
}

uint16_t ntohs(uint16_t netshort) {
    return ((netshort & 0xFF) << 8) | ((netshort >> 8) & 0xFF);
}

uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF) << 24) |
           ((hostlong & 0xFF00) << 8) |
           ((hostlong >> 8) & 0xFF00) |
           ((hostlong >> 24) & 0xFF);
}

uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong);
}

// ============================================================================
// String Utilities
// ============================================================================

static int isdigit(char c) {
    return c >= '0' && c <= '9';
}

static int atoi(const char* str) {
    int result = 0;
    while (*str && isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

static void itoa(int value, char* str, int base) {
    char* p = str;
    char* q = str;
    int tmp;

    do {
        tmp = value % base;
        *p++ = (tmp < 10) ? '0' + tmp : 'a' + tmp - 10;
        value /= base;
    } while (value);

    *p-- = '\0';

    // Reverse string
    while (q < p) {
        char c = *q;
        *q++ = *p;
        *p-- = c;
    }
}

// ============================================================================
// IP Address Conversion
// ============================================================================

uint32_t ip_from_string(const char* str) {
    uint32_t ip = 0;
    int octets[4] = {0, 0, 0, 0};
    int idx = 0;
    int num = 0;

    while (*str && idx < 4) {
        if (isdigit(*str)) {
            num = num * 10 + (*str - '0');
        } else if (*str == '.') {
            octets[idx++] = num;
            num = 0;
        }
        str++;
    }

    if (idx == 3) {
        octets[3] = num;
    }

    ip = (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];
    return ip;
}

void ip_to_string(uint32_t ip, char* buf) {
    char tmp[4];
    int offset = 0;

    for (int i = 3; i >= 0; i--) {
        int octet = (ip >> (i * 8)) & 0xFF;
        itoa(octet, tmp, 10);

        int j = 0;
        while (tmp[j]) {
            buf[offset++] = tmp[j++];
        }

        if (i > 0) {
            buf[offset++] = '.';
        }
    }

    buf[offset] = '\0';
}

int ip_is_valid(const char* str) {
    int dots = 0;
    int digits = 0;
    int num = 0;

    while (*str) {
        if (isdigit(*str)) {
            num = num * 10 + (*str - '0');
            digits++;
            if (num > 255 || digits > 3) return 0;
        } else if (*str == '.') {
            if (digits == 0) return 0;
            dots++;
            digits = 0;
            num = 0;
        } else {
            return 0;
        }
        str++;
    }

    return (dots == 3 && digits > 0);
}

// ============================================================================
// MAC Address Conversion
// ============================================================================

void mac_to_string(const uint8_t* mac, char* buf) {
    const char* hex = "0123456789abcdef";
    int offset = 0;

    for (int i = 0; i < 6; i++) {
        buf[offset++] = hex[(mac[i] >> 4) & 0x0F];
        buf[offset++] = hex[mac[i] & 0x0F];
        if (i < 5) {
            buf[offset++] = ':';
        }
    }

    buf[offset] = '\0';
}

int mac_from_string(const char* str, uint8_t* mac) {
    // TODO: Parse MAC address from string
    return -1;
}

// ============================================================================
// Checksum Calculation
// ============================================================================

uint16_t checksum(const uint8_t* data, uint32_t len) {
    uint32_t sum = 0;
    const uint16_t* ptr = (const uint16_t*)data;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(const uint8_t*)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

uint16_t tcp_udp_checksum(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, const uint8_t* data, uint32_t len) {
    // Build pseudo header
    uint8_t pseudo[12 + len];
    uint8_t* p = pseudo;

    // Source IP
    *p++ = (src_ip >> 24) & 0xFF;
    *p++ = (src_ip >> 16) & 0xFF;
    *p++ = (src_ip >> 8) & 0xFF;
    *p++ = src_ip & 0xFF;

    // Destination IP
    *p++ = (dest_ip >> 24) & 0xFF;
    *p++ = (dest_ip >> 16) & 0xFF;
    *p++ = (dest_ip >> 8) & 0xFF;
    *p++ = dest_ip & 0xFF;

    // Zero
    *p++ = 0;

    // Protocol
    *p++ = protocol;

    // Length
    *p++ = (len >> 8) & 0xFF;
    *p++ = len & 0xFF;

    // Copy data
    for (uint32_t i = 0; i < len; i++) {
        p[i] = data[i];
    }

    return checksum(pseudo, 12 + len);
}
