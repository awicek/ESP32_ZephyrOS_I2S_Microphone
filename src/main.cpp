#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/console/console.h>

#include <stdlib.h>

#include "access_point.h"
#include "sound_recording.h"
#include "sound_queue.h"
static SoundQueue s_queue;
static I2SWrapper sound_wrapper(&s_queue);


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

#define STACK_SIZE 4096  // Stack size for the thread
#define PRIORITY 5       // Thread priority

#define STACK_SIZE_TX 4096  // Stack size for the thread
#define PRIORITY_TX 5       // Thread priority

#define SERVER_IP "192.168.4.11"  // Change to your PC's IP

static int sock;

void identify_caller(void) {
    struct k_thread *current_thread = k_current_get();
    const char *thread_name = k_thread_name_get(current_thread);

    LOG_INF("Function called by thread: %s (ID: %p)\n",
           thread_name ? thread_name : "Unnamed", current_thread);
}

void thread_function(void *arg1, void *arg2, void *arg3)
{
    console_getline_init();
    identify_caller();
    int rc;
    struct sockaddr_in server_addr;
    char msg[1024];
    memset(msg, (int)'a', sizeof(msg));
    
    sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        return;
    LOG_INF("type in a port number: ");
    char *port_str = console_getline();
    int port_int = atoi(port_str);
    LOG_INF("sock %d, p_str %s, p_int%d",sock, port_str, port_int);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_int);
    
    zsock_inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr));

    while (1)
    {
        rc = zsock_connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (rc < 0)
            printk("Failed to connect to server %d \n", rc);
        else 
            break;
        k_sleep(K_MSEC(1000));
    }
    while(true)
    {
        rc = zsock_recv(sock, msg, sizeof(msg), 0);
        if (rc > 0)
        {
            LOG_HEXDUMP_DBG(msg, rc, "received tcp packet");
            if (msg[0] == 's')
                sound_wrapper.startRecording();
            else
            {
                sound_wrapper.stopRecording();
            }   
        }
        else 
        {
            LOG_ERR("recv err rc: %d", rc);
        }
    }   

    zsock_close(sock);;
}


void tx_function(void *arg1, void *arg2, void *arg3)
{
    uint16_t *data;
    while(1)
    {
        s_queue.waitForContainer();

        if (s_queue.getRearContainer(data))
        {
            zsock_send(sock, data, SOUND_Q_SIZE_OF_CONTAINER * 2, 0);
            s_queue.pop();
        }
    }
}


K_THREAD_STACK_DEFINE(my_stack, STACK_SIZE);
static struct k_thread my_thread;
K_THREAD_STACK_DEFINE(my_stack_tx, STACK_SIZE);
static struct k_thread my_thread_tx;


void print_thread_info(const struct k_thread *thread, void *user_data) {
	const char *name = k_thread_name_get((struct k_thread*)thread);
	int priority = k_thread_priority_get((struct k_thread*)thread);

	printk("Thread: %s (ID: %p) | Priority: %d\n",
		   name ? name : "Unnamed", thread, priority);
}


int main(void)
{
    int rc;
    AccessPointEsp32::getInstance().start();
    k_thread_create(&my_thread, my_stack, STACK_SIZE,
		thread_function, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
    k_thread_create(&my_thread_tx, my_stack_tx, STACK_SIZE_TX,
            tx_function, NULL, NULL, NULL,
            PRIORITY_TX, 0, K_NO_WAIT);
    if (sound_wrapper.isReady())
    {
        LOG_INF("i2s is ready");
    }
    
	k_thread_foreach(print_thread_info, NULL);
    return 0;
}