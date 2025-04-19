#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/console/console.h>

#include <stdlib.h>

    
#include "access_point.h"
#include "sound_recording.h"
#include "sound_queue.h"
#include "network_com.h"
static SoundQueue s_queue;
static I2SWrapper sound_wrapper(&s_queue);


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);


#define SERVER_IP "192.168.4.11"  // Change to your PC's IP
#define SERVER_PORT 2803         // Change to your server's port


int main(void)
{
    int rc;
    struct k_sem pc_connected_sem;
    rc = k_sem_init(&pc_connected_sem, 0, 1);
    if (rc != 0)
    {
        LOG_ERR("Failed to initialize semaphore");
        return 0;
    }
    AccessPointEsp32::getInstance().addOnConnectSemaphore(&pc_connected_sem);
    AccessPointEsp32::getInstance().start();
    LOG_INF("Waiting for connection...");

    k_sem_take(&pc_connected_sem, K_FOREVER);
    // k_msleep(5000);
    LOG_INF("Connected");

    if (sound_wrapper.isReady())
    {
        LOG_INF("i2s is ready");
    }
    LOG_INF("Connecting to the server... ");
    // k_msleep(3000);
    // NetworkCom network_com(&s_queue);
    // rc = network_com.connect(SERVER_IP, SERVER_PORT, SERVER_PORT+1);
    // if (rc < 0)
    // {
    //     LOG_INF("connection failed");
    // }
    // else
    // {
    //     LOG_INF("connection success");
    // }

    k_sleep(K_FOREVER);
    return 0;
}