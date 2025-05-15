#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/console/console.h>

#include <stdlib.h>

#include "access_point.h"
#include "sound_recording.h"
#include "sound_queue.h"
#include "network_com.h"
#include "user_buttons.h"
#include "speach.h"
#include "led_controler.h"


#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN, LOG_LEVEL_DBG);

#define SERVER_IP "192.168.4.11"
#define SERVER_PORT 2803           


static SoundQueue s_queue;
static I2SWrapper sound_wrapper(&s_queue);

void waiting(void *arg1, void *arg2, void *arg3);

K_SEM_DEFINE(pc_connected_sem, 0, 1);
K_THREAD_DEFINE(server_connect_th, 4096, waiting, NULL, NULL, NULL, 9, 0, 0);

int main(void)
{
    if (sound_wrapper.isReady())
        LOG_INF("i2s is ready");
    
    LOG_INF("Waiting for pc to connect...");
    AccessPointEsp32::getInstance().addOnConnectSemaphore(&pc_connected_sem);
    AccessPointEsp32::getInstance().start();
    k_sleep(K_FOREVER);
}

void waiting(void *arg1, void *arg2, void *arg3)
{
    int rc;
    k_sem_take(&pc_connected_sem, K_FOREVER);
    LOG_INF("PC connected");
    led_set_mode(LED_WAITING_FOR_SERVER);
    
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
        led_set_mode(LED_READY);
    }

    Speach::getInstance().init();

    UserButtons::getInstance().setNetworkCom(&network_com);
    UserButtons::getInstance().setSoundRecording(&sound_wrapper);
    UserButtons::getInstance().enableButtons(true);
    
    k_sleep(K_FOREVER);
    return;
}