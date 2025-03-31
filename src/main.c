/*
* Copyright (c) 2024 Muhammad Haziq
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/dhcpv4_server.h>
#include <zephyr/console/console.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(MAIN);

#define STACK_SIZE 4096  // Stack size for the thread
#define PRIORITY 0       // Thread priority

static struct k_sem pc_connected;
#define SERVER_IP "192.168.4.12"  // Change to your PC's IP

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
    k_sem_take(&pc_connected, K_FOREVER);
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
    int64_t stop = 0;
    int64_t start = k_uptime_get();
    for (int i = 0; i < 100; )
    {   
        rc = zsock_send(sock, msg, sizeof(msg) , 0);
        if (rc == sizeof(msg))
            ++i;
        printk("%d\n", rc);
    }
    stop = k_uptime_get();
    k_sleep(K_MSEC(3000));
    LOG_INF("time = %lld , stop %lld, start %lld", stop -start, stop, start);
    printk("%lld %lld\n", start, stop);


    zsock_close(sock);
}

K_THREAD_STACK_DEFINE(my_stack, STACK_SIZE);
static struct k_thread my_thread;

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

#define NET_EVENT_WIFI_MASK                                                                    \
    (NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT |                        \
    NET_EVENT_WIFI_AP_ENABLE_RESULT | NET_EVENT_WIFI_AP_DISABLE_RESULT |                      \
    NET_EVENT_WIFI_AP_STA_CONNECTED | NET_EVENT_WIFI_AP_STA_DISCONNECTED)

/* AP Mode Configuration */
#define WIFI_AP_SSID       "ESP32-AP"
#define WIFI_AP_PSK        ""
#define WIFI_AP_IP_ADDRESS "192.168.4.1"
#define WIFI_AP_NETMASK    "255.255.255.0"


static struct k_sem pc_conect_sem; 

static struct net_if *ap_iface;

static struct wifi_connect_req_params ap_config;

static struct net_mgmt_event_callback cb;

static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
                    struct net_if *iface)
{
    switch (mgmt_event) {
    case NET_EVENT_WIFI_AP_ENABLE_RESULT: {
        LOG_INF("AP Mode is enabled. Waiting for station to connect");
        break;
    }
    case NET_EVENT_WIFI_AP_DISABLE_RESULT: {
        LOG_INF("AP Mode is disabled.");
        break;
    }
    case NET_EVENT_WIFI_AP_STA_CONNECTED: {
        struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

        LOG_INF("station: " MACSTR " joined ", sta_info->mac[0], sta_info->mac[1],
            sta_info->mac[2], sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
        k_sem_give(&pc_connected);
        break;
    }
    case NET_EVENT_WIFI_AP_STA_DISCONNECTED: {
        struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;

        LOG_INF("station: " MACSTR " leave ", sta_info->mac[0], sta_info->mac[1],
            sta_info->mac[2], sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
        break;
    }
    default:
        LOG_INF("evend %d ", mgmt_event);
        break;
    }
}

static void enable_dhcpv4_server(void)
{
    static struct in_addr addr;
    static struct in_addr netmaskAddr;

    if (net_addr_pton(AF_INET, WIFI_AP_IP_ADDRESS, &addr)) {
        LOG_ERR("Invalid address: %s", WIFI_AP_IP_ADDRESS);
        return;
    }

    if (net_addr_pton(AF_INET, WIFI_AP_NETMASK, &netmaskAddr)) {
        LOG_ERR("Invalid netmask: %s", WIFI_AP_NETMASK);
        return;
    }

    net_if_ipv4_set_gw(ap_iface, &addr);

    if (net_if_ipv4_addr_add(ap_iface, &addr, NET_ADDR_MANUAL, 0) == NULL) {
        LOG_ERR("unable to set IP address for AP interface");
    }

    if (!net_if_ipv4_set_netmask_by_addr(ap_iface, &addr, &netmaskAddr)) {
        LOG_ERR("Unable to set netmask for AP interface: %s", WIFI_AP_NETMASK);
    }

    addr.s4_addr[3] += 10; /* Starting IPv4 address for DHCPv4 address pool. */

    if (net_dhcpv4_server_start(ap_iface, &addr) != 0) {
        LOG_ERR("DHCP server is not started for desired IP");
        return;
    }

    LOG_INF("DHCPv4 server started...\n");
}

static int enable_ap_mode(void)
{
    if (!ap_iface) {
        LOG_INF("AP: is not initialized");
        return -EIO;
    }

    LOG_INF("Turning on AP Mode");
    ap_config.ssid = (const uint8_t *)WIFI_AP_SSID;
    ap_config.ssid_length = strlen(WIFI_AP_SSID);
    ap_config.psk = (const uint8_t *)WIFI_AP_PSK;
    ap_config.psk_length = strlen(WIFI_AP_PSK);
    ap_config.channel = WIFI_CHANNEL_ANY;
    ap_config.band = WIFI_FREQ_BAND_2_4_GHZ;

    if (strlen(WIFI_AP_PSK) == 0) {
        ap_config.security = WIFI_SECURITY_TYPE_NONE;
    } else {

        ap_config.security = WIFI_SECURITY_TYPE_PSK;
    }

    enable_dhcpv4_server();

    int ret = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, ap_iface, &ap_config,
                sizeof(struct wifi_connect_req_params));
    if (ret) {
        LOG_ERR("NET_REQUEST_WIFI_AP_ENABLE failed, err: %d", ret);
    }

    return ret;
}

void print_thread_info(struct k_thread *thread, void *user_data) {
	const char *name = k_thread_name_get(thread);
	int priority = k_thread_priority_get(thread);

	printk("Thread: %s (ID: %p) | Priority: %d\n",
		   name ? name : "Unnamed", thread, priority);
}


int main(void)
{
    k_sem_init(&pc_connected, 0, 1);
    k_thread_create(&my_thread, my_stack, STACK_SIZE,
		thread_function, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
    // k_sleep(K_SECONDS(5));;
    int rc;
    rc = k_sem_init(&pc_conect_sem, 0, 1);
    if (rc < 0)
    {
        LOG_ERR("nef");
        return -1;
    }

    net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);
    net_mgmt_add_event_callback(&cb);

    ap_iface = net_if_get_wifi_sap();

    enable_ap_mode();


    return 0;
}