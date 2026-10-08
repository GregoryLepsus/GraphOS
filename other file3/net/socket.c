// ============================================================================
// Socket API Implementation
// Berkeley sockets-like interface for network programming
// ============================================================================

#include "../include/socket.h"
#include "../include/tcp.h"
#include "../include/udp.h"
#include "../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Socket table
static socket_t sockets[MAX_SOCKETS];
static int next_fd = 1;

// ============================================================================
// Initialization
// ============================================================================

void socket_init(void) {
    terminal_write_line("[Socket] Initializing socket API...");

    // Clear socket table
    for (int i = 0; i < MAX_SOCKETS; i++) {
        sockets[i].valid = 0;
        sockets[i].fd = 0;
        sockets[i].protocol_data = NULL;
    }

    next_fd = 1;

    terminal_write_line("[Socket] Initialization complete");
}

// ============================================================================
// Find Socket by FD
// ============================================================================

static socket_t* find_socket(int fd) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (sockets[i].valid && sockets[i].fd == fd) {
            return &sockets[i];
        }
    }
    return NULL;
}

// ============================================================================
// Create Socket
// ============================================================================

int socket_create(int family, int type, int protocol) {
    if (family != AF_INET) {
        return -1;  // Only IPv4 supported
    }

    if (type != SOCK_STREAM && type != SOCK_DGRAM) {
        return -1;  // Only TCP and UDP supported
    }

    // Find free socket slot
    socket_t* sock = NULL;
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (!sockets[i].valid) {
            sock = &sockets[i];
            break;
        }
    }

    if (!sock) {
        return -1;  // No free slots
    }

    // Initialize socket
    sock->fd = next_fd++;
    sock->family = family;
    sock->type = type;
    sock->protocol = (type == SOCK_STREAM) ? IPPROTO_TCP : IPPROTO_UDP;
    sock->state = SOCKET_STATE_CLOSED;
    sock->local_ip = 0;
    sock->local_port = 0;
    sock->remote_ip = 0;
    sock->remote_port = 0;
    sock->valid = 1;

    // Allocate protocol-specific data
    if (type == SOCK_STREAM) {
        sock->protocol_data = kmalloc(sizeof(tcp_socket_t));
    } else {
        sock->protocol_data = kmalloc(sizeof(udp_socket_t));
    }

    terminal_write("[Socket] Created socket fd=");
    terminal_write_hex(sock->fd);
    terminal_write(" type=");
    terminal_write_hex(type);
    terminal_write_line("");

    return sock->fd;
}

// ============================================================================
// Bind Socket
// ============================================================================

int socket_bind(int sockfd, uint32_t addr, uint16_t port) {
    socket_t* sock = find_socket(sockfd);
    if (!sock) return -1;

    sock->local_ip = addr;
    sock->local_port = port;
    sock->state = SOCKET_STATE_BOUND;

    // Bind protocol socket
    if (sock->type == SOCK_DGRAM) {
        udp_socket_t* udp_sock = (udp_socket_t*)sock->protocol_data;
        udp_bind(udp_sock, port);
    }

    terminal_write("[Socket] Bound fd=");
    terminal_write_hex(sockfd);
    terminal_write(" to port ");
    terminal_write_hex(port);
    terminal_write_line("");

    return 0;
}

// ============================================================================
// Listen
// ============================================================================

int socket_listen(int sockfd, int backlog) {
    socket_t* sock = find_socket(sockfd);
    if (!sock || sock->type != SOCK_STREAM) return -1;

    if (sock->state != SOCKET_STATE_BOUND) {
        return -1;
    }

    sock->state = SOCKET_STATE_LISTENING;

    terminal_write("[Socket] Listening on fd=");
    terminal_write_hex(sockfd);
    terminal_write_line("");

    return 0;
}

// ============================================================================
// Accept
// ============================================================================

int socket_accept(int sockfd) {
    socket_t* sock = find_socket(sockfd);
    if (!sock || sock->state != SOCKET_STATE_LISTENING) return -1;

    // TODO: Block until incoming connection
    // TODO: Create new socket for accepted connection

    terminal_write("[Socket] Accept on fd=");
    terminal_write_hex(sockfd);
    terminal_write_line("");

    return -1;  // Not implemented yet
}

// ============================================================================
// Connect
// ============================================================================

int socket_connect(int sockfd, uint32_t addr, uint16_t port) {
    socket_t* sock = find_socket(sockfd);
    if (!sock) return -1;

    sock->remote_ip = addr;
    sock->remote_port = port;

    if (sock->type == SOCK_STREAM) {
        // TCP connect
        tcp_socket_t* tcp_sock = (tcp_socket_t*)sock->protocol_data;
        tcp_sock->local_ip = sock->local_ip;
        tcp_sock->remote_ip = addr;
        tcp_sock->remote_port = port;

        if (tcp_connect(tcp_sock, addr, port) < 0) {
            return -1;
        }
    }

    sock->state = SOCKET_STATE_CONNECTED;

    terminal_write("[Socket] Connected fd=");
    terminal_write_hex(sockfd);
    terminal_write(" to ");
    terminal_write_hex((addr >> 24) & 0xFF);
    terminal_write(".");
    terminal_write_hex((addr >> 16) & 0xFF);
    terminal_write(".");
    terminal_write_hex((addr >> 8) & 0xFF);
    terminal_write(".");
    terminal_write_hex(addr & 0xFF);
    terminal_write(":");
    terminal_write_hex(port);
    terminal_write_line("");

    return 0;
}

// ============================================================================
// Send
// ============================================================================

int socket_send(int sockfd, const void* buf, uint32_t len, int flags) {
    socket_t* sock = find_socket(sockfd);
    if (!sock || sock->state != SOCKET_STATE_CONNECTED) return -1;

    if (sock->type == SOCK_STREAM) {
        // TCP send
        tcp_socket_t* tcp_sock = (tcp_socket_t*)sock->protocol_data;
        return tcp_send(tcp_sock, (uint8_t*)buf, len);
    }

    return -1;
}

// ============================================================================
// Receive
// ============================================================================

int socket_recv(int sockfd, void* buf, uint32_t len, int flags) {
    socket_t* sock = find_socket(sockfd);
    if (!sock || sock->state != SOCKET_STATE_CONNECTED) return -1;

    if (sock->type == SOCK_STREAM) {
        // TCP receive
        tcp_socket_t* tcp_sock = (tcp_socket_t*)sock->protocol_data;
        return tcp_receive(tcp_sock, (uint8_t*)buf, len);
    }

    return -1;
}

// ============================================================================
// Send To (UDP)
// ============================================================================

int socket_sendto(int sockfd, const void* buf, uint32_t len, uint32_t dest_addr, uint16_t dest_port) {
    socket_t* sock = find_socket(sockfd);
    if (!sock || sock->type != SOCK_DGRAM) return -1;

    udp_socket_t* udp_sock = (udp_socket_t*)sock->protocol_data;
    return udp_send(udp_sock, dest_addr, dest_port, (uint8_t*)buf, len);
}

// ============================================================================
// Receive From (UDP)
// ============================================================================

int socket_recvfrom(int sockfd, void* buf, uint32_t len, uint32_t* src_addr, uint16_t* src_port) {
    socket_t* sock = find_socket(sockfd);
    if (!sock || sock->type != SOCK_DGRAM) return -1;

    udp_socket_t* udp_sock = (udp_socket_t*)sock->protocol_data;
    return udp_receive(udp_sock, (uint8_t*)buf, len, src_addr, src_port);
}

// ============================================================================
// Close Socket
// ============================================================================

int socket_close(int sockfd) {
    socket_t* sock = find_socket(sockfd);
    if (!sock) return -1;

    // Close protocol socket
    if (sock->type == SOCK_STREAM && sock->state == SOCKET_STATE_CONNECTED) {
        tcp_socket_t* tcp_sock = (tcp_socket_t*)sock->protocol_data;
        tcp_close(tcp_sock);
    }

    // Free protocol data
    if (sock->protocol_data) {
        kfree(sock->protocol_data);
        sock->protocol_data = NULL;
    }

    // Mark socket as free
    sock->valid = 0;

    terminal_write("[Socket] Closed fd=");
    terminal_write_hex(sockfd);
    terminal_write_line("");

    return 0;
}
