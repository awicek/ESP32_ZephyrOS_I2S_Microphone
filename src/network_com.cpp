#include "network_com.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(NetworkCom, LOG_LEVEL_DBG);

#define UDP_PRIORITY 50
#define UDP_STACK_SIZE 4096
K_THREAD_STACK_DEFINE(udp_tx_stack, UDP_STACK_SIZE);

#define TCP_PRIORITY 52
#define TCP_STATCK_SIZE 4096
K_THREAD_STACK_DEFINE(tcp_rx_stack, TCP_STATCK_SIZE);




NetworkCom::NetworkCom(SoundQueue *queue)
    : _data_queue(queue)
{
    k_thread_create(&_udp_tx_thread, udp_tx_stack, UDP_STACK_SIZE,
                    NetworkCom::txLoop, this, NULL, NULL,
                    UDP_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&_tcp_rx_thread, tcp_rx_stack, TCP_STATCK_SIZE,
                    NetworkCom::rxLoop, this, NULL, NULL,
                    TCP_PRIORITY, 0, K_NO_WAIT);    
}

bool NetworkCom::tryConnectSocket(int sock)
{
    int rc;
    for (size_t i = 0; i < SOCKET_RECONNECT_COUNT; i++)
    {
        LOG_INF("connectint to ");
        rc = zsock_connect(sock, (struct sockaddr *)&_server_addr, sizeof(_server_addr));
        if (rc > 0)
            break;
        k_sleep(K_MSEC(SOCKET_RECONNECT_TIMEOUT));
    }
    if (rc < 0)
        return false;
    return true;    
}

int NetworkCom::connect(const char* ip, int port_udp, int port_tcp)
{
    _server_addr.sin_family = AF_INET;
    zsock_inet_pton(AF_INET, ip, &(_server_addr.sin_addr));
    

    LOG_INF("connecting tcp: ip: %s, port_tcp: %d, port_udp: %d", ip, port_tcp, port_udp);
    k_sleep(K_MSEC(1000));
    // tcp_sock
    _server_addr.sin_port = htons(port_tcp);
    _tcp_sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_tcp_sock < 0)
    {
        LOG_ERR("socket tcp failed");
        return -1;
    }
    if (!tryConnectSocket(_tcp_sock))
    {
        LOG_ERR("tryConnectSocket tcp failed");
        return -1;
    }
    
    // udp_sock
    LOG_INF("connecting tcp: ip: %s, port_tcp: %d, port_udp: %d", ip, port_tcp, port_udp);
    _server_addr.sin_port = htons(port_udp);
    _udp_sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
    if (_udp_sock < 0)
    {
        LOG_ERR("socket udp failed");
        return -1;
    }
    if (!tryConnectSocket(_udp_sock))
    {
        LOG_ERR("tryConnectSocket udp failed");
        zsock_close(_udp_sock);
        return -1;
    }

    return 0;
}


void NetworkCom::txLoop(void *network_com_ptr, void *arg2, void *arg3)
{
    while (1)
    {
        k_sleep(K_MSEC(1000));
        // LOG_INF("txLoop");
    }
}

void NetworkCom::rxLoop(void *network_com_ptr, void *arg2, void *arg3)
{
    while (1)
    {
        k_sleep(K_MSEC(1000));
        // LOG_INF("rxLoop");
    }
}