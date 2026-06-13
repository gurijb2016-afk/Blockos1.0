#include "../kernel/types.h"
#include "../kernel/generic.h"
#include "network.h"

/* BlockOS Network Stack */

#define NET_MAX_PACKETS  64
#define NET_BUFFER_SIZE  1518

static network_t g_net;

/* Ethernet frame struktúra */
typedef struct {
    uint8_t  dst_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;
    uint8_t  payload[1500];
} __attribute__((packed)) eth_frame_t;

/* IP fejléc */
typedef struct {
    uint8_t  version_ihl;
    uint8_t  dscp_ecn;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__((packed)) ip_header_t;

/* TCP fejléc */
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset;
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} __attribute__((packed)) tcp_header_t;

/* UDP fejléc */
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

/* Checksum számítás */
static uint16_t net_checksum(void* data, uint32_t size) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;
    while (size > 1) {
        sum += *ptr++;
        size -= 2;
    }
    if (size) sum += *(uint8_t*)ptr;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

/* Hálózat inicializálás */
bool network_init(network_t* net) {
    if (!net) return false;
    memset(net, 0, sizeof(network_t));

    /* MAC cím beállítás */
    net->mac[0] = 0x52;
    net->mac[1] = 0x54;
    net->mac[2] = 0x00;
    net->mac[3] = 0x12;
    net->mac[4] = 0x34;
    net->mac[5] = 0x56;

    /* IP beállítás */
    net->ip      = (192 << 24) | (168 << 16) | (1 << 8) | 100;
    net->netmask = (255 << 24) | (255 << 16) | (255 << 8) | 0;
    net->gateway = (192 << 24) | (168 << 16) | (1 << 8) | 1;
    net->dns     = (8 << 24) | (8 << 16) | (8 << 8) | 8;

    net->connected = false;
    net->packet_count = 0;

    g_net = *net;
    return true;
}

/* IP cím string */
void net_ip_to_str(uint32_t ip, char* buf) {
    snprintf(buf, 16, "%u.%u.%u.%u",
        (ip >> 24) & 0xFF,
        (ip >> 16) & 0xFF,
        (ip >> 8)  & 0xFF,
        ip & 0xFF);
}

/* Csomag küldés */
bool network_send(network_t* net, uint8_t* data, uint32_t size) {
    if (!net || !data || size == 0) return false;
    if (!net->connected) return false;
    net->packets_sent++;
    net->bytes_sent += size;
    return true;
}

/* Csomag fogadás */
bool network_recv(network_t* net, uint8_t* buffer, uint32_t* size) {
    if (!net || !buffer || !size) return false;
    if (!net->connected) return false;
    net->packets_recv++;
    return false;
}

/* Socket kezelés */
int socket_create(int type) {
    for (int i = 0; i < NET_MAX_SOCKETS; i++) {
        if (!g_net.sockets[i].active) {
            g_net.sockets[i].active = true;
            g_net.sockets[i].type = type;
            g_net.sockets[i].id = i;
            return i;
        }
    }
    return -1;
}

bool socket_bind(int sock, uint32_t ip, uint16_t port) {
    if (sock < 0 || sock >= NET_MAX_SOCKETS) return false;
    g_net.sockets[sock].local_ip   = ip;
    g_net.sockets[sock].local_port = port;
    return true;
}

bool socket_connect(int sock, uint32_t ip, uint16_t port) {
    if (sock < 0 || sock >= NET_MAX_SOCKETS) return false;
    g_net.sockets[sock].remote_ip   = ip;
    g_net.sockets[sock].remote_port = port;
    g_net.sockets[sock].connected   = true;
    return true;
}

bool socket_close(int sock) {
    if (sock < 0 || sock >= NET_MAX_SOCKETS) return false;
    memset(&g_net.sockets[sock], 0, sizeof(socket_t));
    return true;
}

int socket_send(int sock, const uint8_t* data, uint32_t size) {
    if (sock < 0 || sock >= NET_MAX_SOCKETS) return -1;
    if (!g_net.sockets[sock].connected) return -1;
    return (int)size;
}

int socket_recv(int sock, uint8_t* buffer, uint32_t size) {
    if (sock < 0 || sock >= NET_MAX_SOCKETS) return -1;
    return 0;
}
