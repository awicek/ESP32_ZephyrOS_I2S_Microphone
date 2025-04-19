#ifndef TCP_COM_H_
#define TCP_COM_H_

#include "sound_queue.h"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>

class NetworkCom
{
    static const int SOCKET_RECONNECT_COUNT = 50;
    static const int SOCKET_RECONNECT_TIMEOUT = 1000; // ms
public:
    NetworkCom(SoundQueue *queue);

    int connect(const char* ip, int port_udp, int port_tcp);

private:
    int _udp_sock;
    int _tcp_sock;

    static void txLoop(void *network_com_ptr, void *arg2, void *arg3);
    static void rxLoop(void *network_com_ptr, void *arg2, void *arg3);
    bool tryConnectSocket(int sock);

    SoundQueue *_data_queue = NULL;
    struct k_thread _udp_tx_thread;
    struct k_thread _tcp_rx_thread;
    
    struct sockaddr_in _server_addr;

    bool _is_connected = false;
};


#endif // TCP_COM_H_