#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/console/console.h>

#include <stdlib.h>

#include "access_point.h"
#include "sound_recording.h"
static I2SWrapper sound_wrapper;
static uint8_t buff[64 * 512];

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

#define STACK_SIZE 4096  // Stack size for the thread
#define PRIORITY 5       // Thread priority

#define SERVER_IP "192.168.4.11"  // Change to your PC's IP
#define SERVER_PORT 1001

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
    int sock;
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
    // int64_t stop = 0;
    // int64_t start = k_uptime_get();
    // for (int i = 0; i < 100; )
    // {   
    //     rc = zsock_send(sock, msg, sizeof(msg) , 0);
    //     if (rc == sizeof(msg))
    //         ++i;
    //     printk("%d\n", rc);
    // }
    // stop = k_uptime_get();
    // k_sleep(K_MSEC(3000));
    // LOG_INF("time = %lld , stop %lld, start %lld", stop -start, stop, start);
    // printk("%lld %lld\n", start, stop);

    while(true)
    {
        rc = zsock_recv(sock, msg, sizeof(msg), 0);
        if (rc > 0)
        {
            LOG_HEXDUMP_DBG(msg, rc, "received tcp packet");
            if (msg[0] == 's')
                sound_wrapper.startRecording();
            else 
                sound_wrapper.stopRecording();
        }
        else 
        {
            LOG_ERR("recv err rc: %d", rc);
        }
    }   

    zsock_close(sock);
}

K_THREAD_STACK_DEFINE(my_stack, STACK_SIZE);
static struct k_thread my_thread;


void print_thread_info(const struct k_thread *thread, void *user_data) {
	const char *name = k_thread_name_get((struct k_thread*)thread);
	int priority = k_thread_priority_get((struct k_thread*)thread);

	printk("Thread: %s (ID: %p) | Priority: %d\n",
		   name ? name : "Unnamed", thread, priority);
}


int main(void)
{
    uint64_t sum;
    memset(buff, 2, sizeof(buff));
    for (int i = 0; i < 100000; ++i)
    {
        sum += buff[i];
    }
    LOG_INF("buff sum %d",sum);
    int rc;
    AccessPointEsp32::getInstance().start();
    k_thread_create(&my_thread, my_stack, STACK_SIZE,
		thread_function, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
    if (sound_wrapper.isReady())
    {
        LOG_INF("i2s is ready");
    }
    
	k_thread_foreach(print_thread_info, NULL);

    return 0;
}