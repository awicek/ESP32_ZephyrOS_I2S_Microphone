#ifndef TCP_COM_H_
#define TCP_COM_H_

#include "sound_queue.h"
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>


// MAP (Micorphone AI Protocol)
constexpr uint8_t MAP_START_RECORDING[] = "start";
constexpr uint8_t MAP_STOP_RECORDING[]  = "stop"; 
constexpr uint8_t MAP_RESPONSE_HEADER[] = "r::";
constexpr uint8_t MAP_RESPONSE_END[] = "::e";

class NetworkCom
{
public:
    NetworkCom(SoundQueue *queue);

    int connect(const char* ip, int port_udp, int port_tcp);

    int startRecording();
    int stopRecording();

private:
    static void txLoop(void *network_com_ptr, void *arg2, void *arg3);
    static void rxLoop(void *network_com_ptr, void *arg2, void *arg3);
    int tryConnectSocket(int sock);
    int sendDataTCP(const uint8_t *data, size_t len);

    SoundQueue *_data_queue = NULL;
    struct k_thread _udp_tx_thread;
    struct k_thread _tcp_rx_thread;
    int _udp_sock = -1;
    int _tcp_sock = -1;

    /* Notifiy txLoop that udp soctek was binded  */
    struct k_sem _udp_socket_sem;  
    /* Notify rxLoop that tcp socket was binded*/
    struct k_sem _tcp_socket_sem;
    
    struct sockaddr_in _server_addr;
};


#endif // TCP_COM_H_