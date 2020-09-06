#ifndef __WIFI_HPP__
#define __WIFI_HPP__

#include <esp_event.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <cJSON.h>

extern "C" {

namespace LDM {
class WiFi {
public:
    WiFi();

    // initialize connection
    esp_err_t init(void);
    esp_err_t init(wifi_config_t *config);
    esp_err_t deinit(void);
    esp_err_t connect(void);
    esp_err_t disconnect(void);
    esp_err_t setConfig(wifi_interface_t interface, wifi_config_t *config);
    esp_err_t getConfig(wifi_interface_t interface, wifi_config_t *config);

    esp_err_t setWiFiMode(wifi_mode_t mode);
    esp_err_t getWiFiMode(wifi_mode_t *mode);
    static bool isConnected(void);

    cJSON *buildJson(void);

private:
    wifi_init_config_t init_config;
    wifi_config_t config;
    wifi_ps_type_t power_save_mode;
    static bool connected;

    esp_netif_t *netif;
    static esp_ip4_addr_t ipv4_address;
    static esp_ip4_addr_t netmask;
    static esp_ip4_addr_t gateway;
    // esp_ip6_addr_t ipv6_address;
    // static esp_ip_addr_t ip_address;

    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
};
}
}

#endif
