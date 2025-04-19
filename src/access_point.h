#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/dhcpv4_server.h>

constexpr const char WIFI_AP_SSID[] =   "ttgo_lora32";
constexpr const char WIFI_AP_PSK[] =    "";

class AccessPointEsp32
{
public:
    static AccessPointEsp32& getInstance();
    void addOnConnectSemaphore(struct k_sem *sem);
    int start();

private:
    AccessPointEsp32() = default;
    AccessPointEsp32 (const AccessPointEsp32&) = delete;
    AccessPointEsp32& operator=(const AccessPointEsp32&) = delete;

    static void wifiEventHandler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
			       struct net_if *iface);
    int enableDhcpv4Server(void);
    int enableApMode(void);

    struct net_if *_ap_iface;
    struct wifi_connect_req_params _ap_config;
    struct net_mgmt_event_callback _cb;
    struct in_addr _addr;
	struct in_addr _netmask_addr;

    static struct k_sem *_on_connect_sem;
};
#endif