#ifndef TCP_COM_H_
#define TCP_COM_H_

#include "sound_queue.h"
#include <zephyr/kernel.h>

class NetworkCom
{
public:
    NetworkCom(SoundQueue *queue);

    void connect(const char* ip, int port);
    // void disconnect();

    void isConnected();

private:
    int udp_sock;
    int tcp_sock;

    SoundQueue *_data_queue = NULL;
    static struct k_thread _udp_tx_thread;
    static struct k_thread _tcp_rx_thread;

    bool _is_connected = false;
};


#endif // TCP_COM_H_