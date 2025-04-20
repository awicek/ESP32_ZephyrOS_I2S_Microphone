#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/console/console.h>

#include <stdlib.h>

    
#include "access_point.h"
#include "sound_recording.h"
#include "sound_queue.h"
#include "network_com.h"
#include "user_buttons.h"

static SoundQueue s_queue;
static I2SWrapper sound_wrapper(&s_queue);


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);


#define SERVER_IP "192.168.4.12"  // Change to your PC's IP
#define SERVER_PORT 2803         // Change to your server's port


#define PRIORITY_WAITING 50
#define STACK_SIZE_WAITING 4096
K_THREAD_STACK_DEFINE(waiting_for_server_stack, STACK_SIZE_WAITING);
static struct k_thread waiting_for_server_thread;

static struct k_sem pc_connected_sem;

void waiting(void *arg1, void *arg2, void *arg3)
{
    int rc;
    k_sem_take(&pc_connected_sem, K_FOREVER);
    LOG_INF("PC connected");
    for (int i = 0; i < 10; ++i)
    {
        LOG_INF("%d", 10-i);
        k_msleep(1000);
    }


    LOG_INF("Connecting to the server... ");
    NetworkCom network_com(&s_queue);
    rc = network_com.connect(SERVER_IP, SERVER_PORT, SERVER_PORT+1);
    if (rc < 0)
    {
        LOG_INF("Connection failed");
    }
    else
    {
        LOG_INF("Connection success");
    }

    UserButtons::getInstance().enableButtons(true);

    k_sleep(K_FOREVER);
    return;
}

int main(void)
{
    int rc;

    rc = k_sem_init(&pc_connected_sem, 0, 1);
    if (rc != 0)
    {
        LOG_ERR("Failed to initialize semaphore");
        return 0;
    }

    k_thread_create(&waiting_for_server_thread, waiting_for_server_stack, STACK_SIZE_WAITING,
                    waiting, NULL, NULL, NULL,
                    PRIORITY_WAITING, 0, K_NO_WAIT);

    if (sound_wrapper.isReady())
        LOG_INF("i2s is ready");
    
    LOG_INF("Waiting for pc to connect...");
    AccessPointEsp32::getInstance().addOnConnectSemaphore(&pc_connected_sem);
    AccessPointEsp32::getInstance().start();
    
    k_sleep(K_FOREVER);
}