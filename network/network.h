#ifndef NETWORK_H
#define NETWORK_H

#include "../kernel/types.h"

#define NET_MAX_SOCKETS 32

/* Socket típusok */
#define SOCK_TCP  0
#define SOCK_UDP  1
#define SOCK_RAW  2

typedef struct {
    bool     active;
    int      id;
    int      type;
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    bool     connected;
} socket_t;

typedef struct {
    uint8_t  mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns;
    bool     connected;
    uint32_t packet_count;
    uint32_t packets_sent;
    uint32_t packets_recv;
    uint32_t bytes_sent;
    uint32_t bytes_recv;
    socket_t sockets[NET_MAX_SOCKETS];
} network_t;

bool network_init(network_t* net);
bool network_send(network_t* net, uint8_t* data, uint32_t size);
bool network_recv(network_t* net, uint8_t* buffer, uint32_t* size);
void net_ip_to_str(uint32_t ip, char* buf);

int  socket_create(int type);
bool socket_bind(int sock, uint32_t ip, uint16_t port);
bool socket_connect(int sock, uint32_t ip, uint16_t port);
bool socket_close(int sock);
int  socket_send(int sock, const uint8_t* data, uint32_t size);
int  socket_recv(int sock, uint8_t* buffer, uint32_t size);

#endif
