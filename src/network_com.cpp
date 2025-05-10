#include "network_com.h"
#include "speach.h"
#include "led_controler.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(NetworkCommunication, LOG_LEVEL_DBG);

#define UDP_PRIORITY 0
#define UDP_STACK_SIZE 4096
K_THREAD_STACK_DEFINE(udp_tx_stack, UDP_STACK_SIZE);

#define TCP_PRIORITY 10
#define TCP_STATCK_SIZE 4096
K_THREAD_STACK_DEFINE(tcp_rx_stack, TCP_STATCK_SIZE);




NetworkCom::NetworkCom(SoundQueue *queue)
    : _data_queue(queue)
{
    if (0 != k_sem_init(&_udp_socket_sem, 0, 1))
        LOG_ERR("sem init error");
    if (0 != k_sem_init(&_tcp_socket_sem, 0, 1))
        LOG_ERR("sem init error");

    k_thread_create(&_udp_tx_thread, udp_tx_stack, UDP_STACK_SIZE,
                    NetworkCom::txLoop, this, NULL, NULL,
                    UDP_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&_tcp_rx_thread, tcp_rx_stack, TCP_STATCK_SIZE,
                    NetworkCom::rxLoop, this, NULL, NULL,
                    TCP_PRIORITY, 0, K_NO_WAIT);    
}

int NetworkCom::tryConnectSocket(int sock)
{
    int rc;
    rc = zsock_connect(sock, (struct sockaddr *)&_server_addr, sizeof(_server_addr));
    LOG_INF("zsock_connect rc: %d", rc);
    return rc;
}

int NetworkCom::connect(const char* ip, int port_udp, int port_tcp)
{
    LOG_INF("connecting tcp: ip: %s, port_tcp: %d, port_udp: %d", ip, port_tcp, port_udp);
    _server_addr.sin_family = AF_INET;
    zsock_inet_pton(AF_INET, ip, &(_server_addr.sin_addr));
    
    // tcp_sock
    _server_addr.sin_port = htons(port_tcp);

    while (1)
    {
        _tcp_sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_tcp_sock < 0)
        {
            k_msleep(1000);
            LOG_ERR("zcok_socket tcp errr");
            continue;
        }
        if (0 == tryConnectSocket(_tcp_sock))
            break;
        
        LOG_ERR("zsock_connect err");
        zsock_close(_tcp_sock);
        k_msleep(1000);
    }
    
    // udp_sock
    LOG_INF("connecting udp");
    _server_addr.sin_port = htons(port_udp);
    while (1)
    {
        _udp_sock = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (_udp_sock < 0)
        {
            k_msleep(1000);
            LOG_ERR("zcok_socket udp errr");
            continue;
        }
        if (0 == tryConnectSocket(_udp_sock))
            break;
        
        LOG_ERR("zsock_connect err");
        zsock_close(_udp_sock);
        k_msleep(1000);
    }

    k_sem_give(&_tcp_socket_sem);
    k_sem_give(&_udp_socket_sem);

    return 0;
}


void NetworkCom::txLoop(void *network_com_ptr, void *arg2, void *arg3)
{
    NetworkCom *this_ptr = static_cast<NetworkCom *>(network_com_ptr);
    uint16_t *data_ptr;

    k_sem_take(&this_ptr->_udp_socket_sem, K_FOREVER);
    LOG_INF("txLoop: udp socket binded");
    
    while (1)
    {
        this_ptr->_data_queue->waitForContainer();
        this_ptr->_data_queue->getRearContainer(data_ptr);
        zsock_send(this_ptr->_udp_sock, data_ptr, SOUND_Q_SIZE_OF_CONTAINER * 2, 0);  // *2 because the datata_ptr is int16_t
        this_ptr->_data_queue->pop();
    }
}

void NetworkCom::rxLoop(void *network_com_ptr, void *arg2, void *arg3)
{
    NetworkCom *this_ptr = static_cast<NetworkCom *>(network_com_ptr);
    
    k_sem_take(&this_ptr->_tcp_socket_sem, K_FOREVER);
    LOG_INF("rxLoop: tcp socket binded");

    uint8_t msg_buf[256];
    int rc;

    while (1)
    {
        rc = zsock_recv(this_ptr->_tcp_sock, msg_buf, sizeof(msg_buf), 0);
        if (rc > 0)
        {
            LOG_HEXDUMP_INF(msg_buf, (uint32_t)rc, "rxLoop: msg");
            msg_buf[rc] = '\0';
            Speach::getInstance().speak((char*)msg_buf);
            led_set_mode(LED_READY);
        }
        
    }
}


int NetworkCom::startRecording()
{
    return sendDataTCP(MAP_START_RECORDING, sizeof(MAP_START_RECORDING));
}


int NetworkCom::stopRecording()
{
    return sendDataTCP(MAP_STOP_RECORDING, sizeof(MAP_STOP_RECORDING));
}


int NetworkCom::sendDataTCP(const uint8_t *data, size_t len)
{
    ssize_t rc = zsock_send(_tcp_sock, data, len, 0);
    if (rc > 0)
        return 0;
    return -1;
}