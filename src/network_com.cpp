#include "network_com.h"

#define UDP_PRIORITY 1;
#define UDP_STACK_SIZE 4096;
K_THREAD_STACK_DEFINE(udp_tx_stack, STACK_SIZE);

#define TCP_PRIORITY 2;
#define TCP_STATCK_SIZE 4096;
K_THREAD_STACK_DEFINE(tcp_rx_stack, STACK_SIZE);




static struct k_thread _udp_tx_thread;
static struct k_thread _tcp_rx_thread;


