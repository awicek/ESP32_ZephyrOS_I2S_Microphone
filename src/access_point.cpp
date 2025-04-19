#include "access_point.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(AccessPointEsp32);

#define NET_EVENT_WIFI_MASK                                                                    \
	(NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT |                        \
	 NET_EVENT_WIFI_AP_ENABLE_RESULT | NET_EVENT_WIFI_AP_DISABLE_RESULT |                      \
	 NET_EVENT_WIFI_AP_STA_CONNECTED | NET_EVENT_WIFI_AP_STA_DISCONNECTED)
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

struct k_sem *AccessPointEsp32::_on_connect_sem = NULL;


AccessPointEsp32& AccessPointEsp32::getInstance()
{
	static AccessPointEsp32 instance;
    return instance;
}

int AccessPointEsp32::start()
{
	net_mgmt_init_event_callback(&_cb,
					wifiEventHandler,
	 				NET_EVENT_WIFI_MASK);
	net_mgmt_add_event_callback(&_cb);

	/* Get AP interface in AP-STA mode. */
	_ap_iface = net_if_get_wifi_sap();

	return enableApMode();
}

void AccessPointEsp32::wifiEventHandler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
			    struct net_if *iface)
{
	switch (mgmt_event)
	{
	case NET_EVENT_WIFI_AP_ENABLE_RESULT:
	{
		LOG_INF("AP Mode is enabled. Waiting for station to connect");
		break;
	}
	case NET_EVENT_WIFI_AP_DISABLE_RESULT:
	{
		LOG_INF("AP Mode is disabled.");
		break;
	}
	case NET_EVENT_WIFI_AP_STA_CONNECTED:
	{
		struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;
		LOG_INF("station: " MACSTR " joined ", sta_info->mac[0], sta_info->mac[1],
			sta_info->mac[2], sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
		if (!k_is_in_isr()) {
			k_sem_give(_on_connect_sem);
			LOG_ERR("not isr");

		} else {
			LOG_ERR("Tried to give semaphore from ISR!");
		}
		break;
	}
	case NET_EVENT_WIFI_AP_STA_DISCONNECTED:
	{
		struct wifi_ap_sta_info *sta_info = (struct wifi_ap_sta_info *)cb->info;
		LOG_INF("station: " MACSTR " leave ", sta_info->mac[0], sta_info->mac[1],
			sta_info->mac[2], sta_info->mac[3], sta_info->mac[4], sta_info->mac[5]);
		break;
	}
	default:
        LOG_INF("wifiEventHandler unknown event: %d", mgmt_event);
		break;
	}
};

int AccessPointEsp32::enableDhcpv4Server(void)
{
	if (net_addr_pton(AF_INET, CONFIG_NET_CONFIG_MY_IPV4_ADDR, &_addr))
	{
		LOG_ERR("Invalid address: %s", CONFIG_NET_CONFIG_MY_IPV4_ADDR);
		return -1;
	}

	if (net_addr_pton(AF_INET, CONFIG_NET_CONFIG_MY_IPV4_NETMASK , &_netmask_addr))
	{
		LOG_ERR("Invalid netmask: %s", CONFIG_NET_CONFIG_MY_IPV4_NETMASK);
		return -1;
	}

	net_if_ipv4_set_gw(_ap_iface, &_addr);

	if (net_if_ipv4_addr_add(_ap_iface, &_addr, NET_ADDR_MANUAL, 0) == NULL)
	{
		LOG_ERR("unable to set IP address for AP interface");
		return -1;
	}

	if (!net_if_ipv4_set_netmask_by_addr(_ap_iface, &_addr, &_netmask_addr))
	{
		LOG_ERR("Unable to set netmask for AP interface: %s", CONFIG_NET_CONFIG_MY_IPV4_NETMASK);
		return -1;
	}

	_addr.s4_addr[3] += 10; // Starting IPv4 address for DHCPv4 address pool.

	if (net_dhcpv4_server_start(_ap_iface, &_addr) != 0)
	{
		LOG_ERR("DHCP server is not started for desired IP");
		return -1;
	}

	LOG_INF("DHCPv4 server started...\n");
	return 0;
};

int AccessPointEsp32::enableApMode(void)
{
    if (!_ap_iface)
    {
        LOG_ERR("AP: is not initialized");
        return -1;
    }

    LOG_INF("Turning on AP Mode");
    _ap_config.ssid = (const uint8_t *)WIFI_AP_SSID;
	_ap_config.ssid_length = strlen(WIFI_AP_SSID);
	_ap_config.psk = (const uint8_t *)WIFI_AP_PSK;
	_ap_config.psk_length = strlen(WIFI_AP_PSK);
	_ap_config.channel = WIFI_CHANNEL_ANY;
	_ap_config.band = WIFI_FREQ_BAND_2_4_GHZ;


	if (strlen(WIFI_AP_PSK) == 0)
    {
		_ap_config.security = WIFI_SECURITY_TYPE_NONE;
	}
    else
    {
		_ap_config.security = WIFI_SECURITY_TYPE_PSK;
	}

    if (0 != enableDhcpv4Server())
    {
        LOG_ERR("enableDhcpv4Server");
        return -1;
    }

    int mgmt_res = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, _ap_iface, &_ap_config,
			   sizeof(struct wifi_connect_req_params));
	if (mgmt_res) {
		LOG_ERR("NET_REQUEST_WIFI_AP_ENABLE failed, err: %d", mgmt_res);
        return -1;
	}

	return 0;
};


void AccessPointEsp32::addOnConnectSemaphore(struct k_sem *sem)
{
	_on_connect_sem = sem;
}