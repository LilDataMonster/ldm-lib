#include <cstring>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <cJSON.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_err.h>

// project headers
#include <wifi.hpp>

#define WIFI_TAG "LDM-LIB:WIFI"

#define ERR_CHECK(_x, _msg) \
if(_x != ESP_OK) {\
    ESP_LOGE(WIFI_TAG, "%s "#_msg": %s\n", __func__, esp_err_to_name(err));\
    return err;\
}

#define CASE_STRING_MACRO(_cjson, _name, _macro) case _macro: \
    cJSON_AddStringToObject(_cjson, _name, #_macro); \
    break;

#define DEFAULT_STA_SSID CONFIG_WIFI_STA_SSID
#define DEFAULT_STA_PWD CONFIG_WIFI_STA_PASSWORD
#define DEFAULT_STA_HOSTNAME CONFIG_WIFI_STA_HOSTNAME

#define DEFAULT_AP_SSID CONFIG_WIFI_AP_SSID
#define DEFAULT_AP_PWD CONFIG_WIFI_AP_PASSWORD
#define DEFAULT_AP_MAX_CONNECTIONS CONFIG_WIFI_AP_MAX_CONNECTIONS
#define DEFAULT_AP_CHANNEL CONFIG_WIFI_AP_CHANNEL
#define DEFAULT_AP_HOSTNAME CONFIG_WIFI_AP_HOSTNAME
#define DEFAULT_AP_IP CONFIG_WIFI_AP_IPV4
#define DEFAULT_AP_GATEWAY CONFIG_WIFI_AP_GATEWAY
#define DEFAULT_AP_NETMASK CONFIG_WIFI_AP_NETMASK

#if CONFIG_WIFI_AUTH_WEP
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WEP
#elif CONFIG_WIFI_AUTH_WPA_PSK
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WPA_PSK
#elif CONFIG_WIFI_AUTH_WPA2_PSK
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA_WPA2_PSK
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA2_ENTERPRISE
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WPA2_ENTERPRISE
#elif CONFIG_WIFI_AUTH_WPA3_PSK
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WPA2_WPA3_PSK
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WAPI_PSK
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_WAPI_PSK
#else
#define DEFAULT_AP_AUTH_MODE WIFI_AUTH_OPEN
#endif

#if CONFIG_POWER_SAVE_MIN_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MIN_MODEM
#elif CONFIG_POWER_SAVE_MAX_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MAX_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif /*CONFIG_POWER_SAVE_MODEM*/

// /* The event group allows multiple bits for each event, but we only care about two events:
//  * - we are connected to the AP with an IP
//  * - we failed to connect after the maximum amount of retries */
// #define WIFI_CONNECTED_BIT BIT0
// #define WIFI_FAIL_BIT      BIT1
const int CONNECTED_BIT = BIT0;

#define MAX_RETRY      5 //CONFIG_ESP_MAXIMUM_RETRY

// /* FreeRTOS event group to signal when we are connected*/
// static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

esp_ip4_addr_t LDM::WiFi::ipv4_address;
esp_ip4_addr_t LDM::WiFi::netmask;
esp_ip4_addr_t LDM::WiFi::gateway;

static bool gl_sta_connected = false;
// static bool ble_is_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;

bool LDM::WiFi::connected = false;
bool LDM::WiFi::hosting = false;
LDM::WiFi::WiFi() {
    //
    this->sta_config = {};
    this->ap_config = {};
    this->power_save_mode = DEFAULT_PS_MODE;


    // setup default ssid/password for station mode
    std::strcpy((char*)this->sta_config.sta.ssid, DEFAULT_STA_SSID);
    std::strcpy((char*)this->sta_config.sta.password, DEFAULT_STA_PWD);

    // setup default ssid/password for ap mode
    std::strcpy((char*)this->ap_config.ap.ssid, DEFAULT_AP_SSID);
    std::strcpy((char*)this->ap_config.ap.password, DEFAULT_AP_PWD);
    this->ap_config.ap.ssid_len = std::strlen(DEFAULT_AP_SSID);
    this->ap_config.ap.authmode = DEFAULT_AP_AUTH_MODE;
    this->ap_config.ap.max_connection = DEFAULT_AP_MAX_CONNECTIONS;
    this->ap_config.ap.channel = DEFAULT_AP_CHANNEL;
#ifdef CONFIG_WIFI_AP_HIDDEN
    this->ap_config.ap.ssid_hidden = 1;
#endif
}

esp_err_t LDM::WiFi::init(WiFiSetup setup) {
    esp_err_t err = ESP_OK;

    // s_wifi_event_group = xEventGroupCreate();

    // init netif
    ESP_ERROR_CHECK(esp_netif_init());

    // create event group
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    if(setup == WiFiSetup::STA || setup == WiFiSetup::APSTA) {
        this->netif_sta = esp_netif_create_default_wifi_sta();
        assert(this->netif_sta);
        err |= esp_netif_set_hostname(this->netif_sta, DEFAULT_STA_HOSTNAME);
    }
    if(setup == WiFiSetup::AP || setup == WiFiSetup::APSTA) {
        this->netif_ap = esp_netif_create_default_wifi_ap();
        assert(this->netif_ap);
        err |= esp_netif_set_hostname(this->netif_ap, DEFAULT_AP_HOSTNAME);

        // set static ap addresses
        esp_netif_ip_info_t ip_info;
        esp_netif_str_to_ip4(DEFAULT_AP_IP, &ip_info.ip);
        esp_netif_str_to_ip4(DEFAULT_AP_GATEWAY, &ip_info.gw);
        esp_netif_str_to_ip4((char*)DEFAULT_AP_NETMASK, &ip_info.netmask);

        ESP_ERROR_CHECK(esp_netif_dhcps_stop(this->netif_ap)); // shutdown dhcp server (TODO: restart this?)
        ESP_ERROR_CHECK(esp_netif_set_ip_info(this->netif_ap, &ip_info));
    }

    // init wifi
    init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&this->init_config));

    // add event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

    // setup wifi mode
    switch(setup) {
        case WiFiSetup::STA: {
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &this->sta_config));
            break;
        }
        case WiFiSetup::AP: {
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
            ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_AP, &this->ap_config));
            break;
        }
        case WiFiSetup::APSTA: {
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
            ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &this->sta_config));
            ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_AP, &this->ap_config));
            break;
        }
    }
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &this->config));

    // start wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    /* init wifi as sta and set power save mode */
    ESP_LOGI(WIFI_TAG, "Setting wifi power save mode");
    esp_wifi_set_ps(this->power_save_mode);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    // ESP_LOGI(WIFI_TAG, "Waiting for event bits");
    // EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
    //     WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    //     pdFALSE,
    //     pdFALSE,
    //     portMAX_DELAY);
    //
    // /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
    // if(bits & WIFI_CONNECTED_BIT) {
    //     ESP_LOGI(WIFI_TAG, "Connected to AP SSID:%s password:%s",
    //         DEFAULT_SSID, DEFAULT_PWD);
    // } else if(bits & WIFI_FAIL_BIT) {
    //     ESP_LOGE(WIFI_TAG, "Failed to connect to SSID:%s, password:%s",
    //         DEFAULT_SSID, DEFAULT_PWD);
    //     err = ESP_FAIL;
    // } else {
    //     ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    //     err = ESP_ERR_INVALID_STATE;
    // }

    // /* The event will not be processed after unregister */
    // ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    // vEventGroupDelete(s_wifi_event_group);
    return err;
}

esp_err_t LDM::WiFi::deinit(void) {
    esp_err_t err = ESP_OK;

    // vEventGroupDelete(s_wifi_event_group);

    err |= esp_wifi_stop();
    if(err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "%s Stopping WiFi failed: %s\n", __func__, esp_err_to_name(err));
        return err;
    }
    err |= esp_wifi_deinit();
    if(err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "%s Deinitialize WiFi failed: %s\n", __func__, esp_err_to_name(err));
        return err;
    }
    err |= esp_netif_deinit();
    if(err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "%s Deinitialize Network Stack failed: %s\n", __func__, esp_err_to_name(err));
        return err;
    }

    if(this->netif_sta != NULL) {
        err |= esp_wifi_clear_default_wifi_driver_and_handlers(this->netif_sta);
        esp_netif_destroy(this->netif_sta);
    }
    if(this->netif_ap != NULL) {
        err |= esp_wifi_clear_default_wifi_driver_and_handlers(this->netif_ap);
        esp_netif_destroy(this->netif_ap);
    }

    LDM::WiFi::connected = false;
    return err;
}

esp_err_t LDM::WiFi::connect(void) {
    esp_err_t err = esp_wifi_connect();
    if(err != ESP_OK) {
        LDM::WiFi::connected = false;
    }
    LDM::WiFi::connected = true;
    return err;
}

esp_err_t LDM::WiFi::disconnect() {
    LDM::WiFi::connected = false;
    return esp_wifi_disconnect();
}

esp_err_t LDM::WiFi::setConfig(wifi_interface_t interface, wifi_config_t *config) {
    esp_err_t err = ESP_OK;
    err = esp_wifi_set_config(interface, config);
    if(err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "%s Setting WiFi Config failed: %s\n", __func__, esp_err_to_name(err));\
        return err;
    }
    return err;
}

esp_err_t LDM::WiFi::getConfig(wifi_interface_t interface, wifi_config_t *config) {
    esp_err_t err = ESP_OK;
    err = esp_wifi_get_config(interface, config);
    if(err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "%s Getting WiFi Config failed: %s\n", __func__, esp_err_to_name(err));\
        return err;
    }
    return err;
}

void LDM::WiFi::wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    switch(event_id) {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_STOP:
        LDM::WiFi::connected = false;
        break;
    case WIFI_EVENT_STA_CONNECTED: {
        gl_sta_connected = true;
        wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*) event_data;
        std::memcpy(gl_sta_bssid, event->bssid, 6);
        std::memcpy(gl_sta_ssid, event->ssid, event->ssid_len);
        gl_sta_ssid_len = event->ssid_len;
        break;
    }
    case WIFI_EVENT_STA_DISCONNECTED: {
        /* This is a workaround as ESP32 WiFi libs don't currently
        auto-reassociate. */
        gl_sta_connected = false;
        std::memset(gl_sta_ssid, 0, 32);
        std::memset(gl_sta_bssid, 0, 6);
        gl_sta_ssid_len = 0;
        esp_wifi_connect();
        // xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
        break;
    }
    case WIFI_EVENT_AP_START:
        LDM::WiFi::hosting = true;
        break;
    case WIFI_EVENT_AP_STOP:
        LDM::WiFi::hosting = false;
        break;
    case WIFI_EVENT_AP_STACONNECTED: {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
        break;
    }
    case WIFI_EVENT_AP_STADISCONNECTED: {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(WIFI_TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
        break;
    }

    // case WIFI_EVENT_AP_START:
    //     esp_wifi_get_mode(&mode);
    //
    //     /* TODO: get config or information of softap, then set to report extra_info */
    //     if(ble_is_connected == true) {
    //         if(gl_sta_connected) {
    //             esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, NULL);
    //         } else {
    //             esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
    //         }
    //     } else {
    //         ESP_LOGI(WIFI_TAG, "BLUFI BLE is not connected yet");
    //     }
    //     break;
    // case WIFI_EVENT_SCAN_DONE: {
    //     uint16_t apCount = 0;
    //     esp_wifi_scan_get_ap_num(&apCount);
    //     if(apCount == 0) {
    //         BLUFI_INFO("Nothing AP found");
    //         break;
    //     }
    //     wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
    //     if(!ap_list) {
    //         BLUFI_ERROR("malloc error, ap_list is NULL");
    //         break;
    //     }
    //     ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
    //     esp_blufi_ap_record_t * blufi_ap_list = (esp_blufi_ap_record_t *)malloc(apCount * sizeof(esp_blufi_ap_record_t));
    //     if(!blufi_ap_list) {
    //         if(ap_list) {
    //             free(ap_list);
    //         }
    //         BLUFI_ERROR("malloc error, blufi_ap_list is NULL");
    //         break;
    //     }
    //     for(int i = 0; i < apCount; ++i) {
    //         blufi_ap_list[i].rssi = ap_list[i].rssi;
    //         memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
    //     }
    //
    //     if(ble_is_connected == true) {
    //         esp_blufi_send_wifi_list(apCount, blufi_ap_list);
    //     } else {
    //         BLUFI_INFO("BLUFI BLE is not connected yet");
    //     }
    //
    //     esp_wifi_scan_stop();
    //     free(ap_list);
    //     free(blufi_ap_list);
    //     break;
    // }
    default:
        break;
    }
    return;
}

void LDM::WiFi::ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    // wifi_mode_t mode;

    switch(event_id) {
    case IP_EVENT_STA_GOT_IP: {
        // esp_blufi_extra_info_t info;
        //
        // xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        // esp_wifi_get_mode(&mode);
        //
        // memset(&info, 0, sizeof(esp_blufi_extra_info_t));
        // memcpy(info.sta_bssid, gl_sta_bssid, 6);
        // info.sta_bssid_set = true;
        // info.sta_ssid = gl_sta_ssid;
        // info.sta_ssid_len = gl_sta_ssid_len;
        // if (ble_is_connected == true) {
        //     esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
        // } else {
        //     // BLUFI_INFO("BLUFI BLE is not connected yet");
        //     ESP_LOGI(WIFI_TAG, "BLUFI BLE is not connected yet");
        // }
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(WIFI_TAG, "Netmask IP: " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(WIFI_TAG, "Gateway IP: " IPSTR, IP2STR(&event->ip_info.gw));

        LDM::WiFi::ipv4_address = *(&event->ip_info.ip);
        LDM::WiFi::netmask = *(&event->ip_info.netmask);
        LDM::WiFi::gateway = *(&event->ip_info.gw);

        LDM::WiFi::connected = true;
        s_retry_num = 0;
        break;
    }
    default:
        break;
    }
    return;
}

void LDM::WiFi::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(WIFI_TAG, "Attempting to connect to AP");
        esp_wifi_connect();
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        if(s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIFI_TAG, "Retrying to connect to the AP");
        } else {
            // xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(WIFI_TAG, "Connecting to the AP failed");
            s_retry_num = 0;
        }
    } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(WIFI_TAG, "Netmask IP: " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(WIFI_TAG, "Gateway IP: " IPSTR, IP2STR(&event->ip_info.gw));

        LDM::WiFi::ipv4_address = *(&event->ip_info.ip);
        LDM::WiFi::netmask = *(&event->ip_info.netmask);
        LDM::WiFi::gateway = *(&event->ip_info.gw);

        LDM::WiFi::connected = true;
        s_retry_num = 0;
        // xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t LDM::WiFi::setWiFiMode(wifi_mode_t mode) {
    esp_err_t err = esp_wifi_set_mode(mode);
    ERR_CHECK(err, "Unable to set WiFi Mode");
    return err;
}

esp_err_t LDM::WiFi::getWiFiMode(wifi_mode_t *mode) {
    esp_err_t err = esp_wifi_get_mode(mode);
    ERR_CHECK(err, "Unable to get WiFi Mode");
    return err;
}

bool LDM::WiFi::isConnected(void) {
    return LDM::WiFi::connected;
}

bool LDM::WiFi::isHosting(void) {
    return LDM::WiFi::hosting;
}

esp_err_t LDM::WiFi::getIpv4(uint8_t* ipv4) {
    if(isConnected()) {
        ipv4[0] = esp_ip4_addr_get_byte(&LDM::WiFi::ipv4_address, 0);
        ipv4[1] = esp_ip4_addr_get_byte(&LDM::WiFi::ipv4_address, 1);
        ipv4[2] = esp_ip4_addr_get_byte(&LDM::WiFi::ipv4_address, 2);
        ipv4[3] = esp_ip4_addr_get_byte(&LDM::WiFi::ipv4_address, 3);
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}

cJSON * LDM::WiFi::buildJson(void) {
    cJSON *json_root = cJSON_CreateObject();

    // build mode info
    wifi_mode_t mode;
    esp_err_t err = LDM::WiFi::getWiFiMode(&mode);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "mode", "NONE");
    } else {
        switch(mode) {
            CASE_STRING_MACRO(json_root, "mode", WIFI_MODE_NULL)
            CASE_STRING_MACRO(json_root, "mode", WIFI_MODE_STA)
            CASE_STRING_MACRO(json_root, "mode", WIFI_MODE_AP)
            CASE_STRING_MACRO(json_root, "mode", WIFI_MODE_APSTA)
            CASE_STRING_MACRO(json_root, "mode", WIFI_MODE_MAX)
        }
    }

    wifi_config_t config;

    // build sta config info
    cJSON *json_wifi_sta_config = cJSON_CreateObject();
    err = esp_wifi_get_config(WIFI_IF_STA, &config);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "config_sta_mode", "NONE");
    } else {
        char bssid_string[18];
        cJSON_AddStringToObject(json_wifi_sta_config, "ssid", (char*)config.sta.ssid);
        cJSON_AddStringToObject(json_wifi_sta_config, "password", (char*)config.sta.password);

        cJSON_AddItemToObject(json_wifi_sta_config, "bssid_set", cJSON_CreateBool(config.sta.bssid_set));

        snprintf(bssid_string, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                 config.sta.bssid[0], config.sta.bssid[1], config.sta.bssid[2],
                 config.sta.bssid[3], config.sta.bssid[4], config.sta.bssid[5]);
        cJSON_AddStringToObject(json_wifi_sta_config, "bssid", bssid_string);
        cJSON_AddNumberToObject(json_wifi_sta_config, "channel", config.sta.channel);
        cJSON_AddNumberToObject(json_wifi_sta_config, "listen_interval", config.sta.listen_interval);

        cJSON_AddItemToObject(json_root, "config_sta_mode", json_wifi_sta_config);
    }

    // build ap config info
    cJSON *json_wifi_ap_config = cJSON_CreateObject();
    err = esp_wifi_get_config(WIFI_IF_AP, &config);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "config_ap_mode", "NONE");
    } else {
        cJSON_AddStringToObject(json_wifi_ap_config, "ssid", (char*)config.ap.ssid);
        cJSON_AddStringToObject(json_wifi_ap_config, "password", (char*)config.ap.password);
        cJSON_AddNumberToObject(json_wifi_ap_config, "ssid_len", config.ap.ssid_len);
        switch(config.ap.authmode) {
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_OPEN)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WEP)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WPA_PSK)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WPA2_PSK)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WPA_WPA2_PSK)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WPA2_ENTERPRISE)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WPA3_PSK)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WPA2_WPA3_PSK)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_WAPI_PSK)
            CASE_STRING_MACRO(json_wifi_ap_config, "authmode", WIFI_AUTH_MAX)
        }
        cJSON_AddNumberToObject(json_wifi_ap_config, "channel", config.ap.channel);
        cJSON_AddNumberToObject(json_wifi_ap_config, "ssid_hidden", config.ap.ssid_hidden);
        cJSON_AddNumberToObject(json_wifi_ap_config, "max_connection", config.ap.max_connection);
        cJSON_AddNumberToObject(json_wifi_ap_config, "beacon_interval", config.ap.beacon_interval);

        cJSON_AddItemToObject(json_root, "config_ap_mode", json_wifi_ap_config);
    }

    // build wifi country info
    cJSON *json_wifi_country = cJSON_CreateObject();
    wifi_country_t country;
    err = esp_wifi_get_country(&country);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "country", "NONE");
    } else {
        cJSON_AddStringToObject(json_wifi_country, "country_code", country.cc);
        cJSON_AddNumberToObject(json_wifi_country, "start_channel", country.schan);
        cJSON_AddNumberToObject(json_wifi_country, "total_channel_num", country.nchan);
        cJSON_AddNumberToObject(json_wifi_country, "max_tx_power", country.max_tx_power);
        switch(country.policy) {
            CASE_STRING_MACRO(json_wifi_country, "country_policy", WIFI_COUNTRY_POLICY_AUTO)
            CASE_STRING_MACRO(json_wifi_country, "country_policy", WIFI_COUNTRY_POLICY_MANUAL)
        }

        cJSON_AddItemToObject(json_root, "country", json_wifi_country);
    }

    // build wifi bandwidth info
    wifi_bandwidth_t bw;
    err = esp_wifi_get_bandwidth(WIFI_IF_STA, &bw);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "sta_bandwidth", "NONE");
    } else {
        switch(bw) {
            CASE_STRING_MACRO(json_root, "sta_bandwidth", WIFI_BW_HT20)
            CASE_STRING_MACRO(json_root, "sta_bandwidth", WIFI_BW_HT40)
        }
    }

    err = esp_wifi_get_bandwidth(WIFI_IF_AP, &bw);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "ap_bandwidth", "NONE");
    } else {
        switch(bw) {
            CASE_STRING_MACRO(json_root, "ap_bandwidth", WIFI_BW_HT20)
            CASE_STRING_MACRO(json_root, "ap_bandwidth", WIFI_BW_HT40)
        }
    }

    // build power save info
    wifi_ps_type_t type;
    err = esp_wifi_get_ps(&type);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "power_save", "NONE");
    } else {
        switch(type) {
            CASE_STRING_MACRO(json_root, "power_save", WIFI_PS_NONE)
            CASE_STRING_MACRO(json_root, "power_save", WIFI_PS_MIN_MODEM)
            CASE_STRING_MACRO(json_root, "power_save", WIFI_PS_MAX_MODEM)
        }
    }

    // build connected ap info
    wifi_ap_record_t ap_info;
    cJSON *json_wifi_ap_info = cJSON_CreateObject();
    err = esp_wifi_sta_get_ap_info(&ap_info);
    if(err != ESP_OK) {
        cJSON_AddStringToObject(json_root, "connected_ap", "NONE");
    } else {
        char bssid_string[18];
        snprintf(bssid_string, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                 ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
                 ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);
        cJSON_AddStringToObject(json_wifi_ap_info, "bssid", bssid_string);
        cJSON_AddStringToObject(json_wifi_ap_info, "ssid", (char*)ap_info.ssid);
        cJSON_AddNumberToObject(json_wifi_ap_info, "primary_channel", ap_info.primary);
        switch(ap_info.second) {
            CASE_STRING_MACRO(json_wifi_ap_info, "secondary_channel", WIFI_SECOND_CHAN_NONE)
            CASE_STRING_MACRO(json_wifi_ap_info, "secondary_channel", WIFI_SECOND_CHAN_ABOVE)
            CASE_STRING_MACRO(json_wifi_ap_info, "secondary_channel", WIFI_SECOND_CHAN_BELOW)
        }
        cJSON_AddNumberToObject(json_wifi_ap_info, "rssi", ap_info.rssi);
        switch(ap_info.authmode) {
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_OPEN)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WEP)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WPA_PSK)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WPA2_PSK)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WPA_WPA2_PSK)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WPA2_ENTERPRISE)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WPA3_PSK)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WPA2_WPA3_PSK)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_WAPI_PSK)
            CASE_STRING_MACRO(json_wifi_ap_info, "authmode", WIFI_AUTH_MAX)
        }
        switch(ap_info.pairwise_cipher) {
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_NONE)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_WEP40)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_WEP104)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_TKIP)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_CCMP)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_TKIP_CCMP)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_AES_CMAC128)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_SMS4)
            CASE_STRING_MACRO(json_wifi_ap_info, "pairwise_cipher", WIFI_CIPHER_TYPE_UNKNOWN)
        }
        switch(ap_info.group_cipher) {
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_NONE)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_WEP40)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_WEP104)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_TKIP)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_CCMP)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_TKIP_CCMP)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_AES_CMAC128)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_SMS4)
            CASE_STRING_MACRO(json_wifi_ap_info, "group_cipher", WIFI_CIPHER_TYPE_UNKNOWN)
        }
        switch(ap_info.ant) {
            CASE_STRING_MACRO(json_wifi_ap_info, "antenna", WIFI_ANT_ANT0)
            CASE_STRING_MACRO(json_wifi_ap_info, "antenna", WIFI_ANT_ANT1)
            CASE_STRING_MACRO(json_wifi_ap_info, "antenna", WIFI_ANT_MAX)
        }

        cJSON_AddNumberToObject(json_wifi_ap_info, "phy_11b", ap_info.phy_11b);
        cJSON_AddNumberToObject(json_wifi_ap_info, "phy_11g", ap_info.phy_11g);
        cJSON_AddNumberToObject(json_wifi_ap_info, "phy_11n", ap_info.phy_11n);
        cJSON_AddNumberToObject(json_wifi_ap_info, "phy_lr", ap_info.phy_lr);
        cJSON_AddNumberToObject(json_wifi_ap_info, "wps", ap_info.wps);
        cJSON_AddNumberToObject(json_wifi_ap_info, "reserved", ap_info.reserved);

        cJSON *json_wifi_ap_country_info = cJSON_CreateObject();
        cJSON_AddStringToObject(json_wifi_ap_country_info, "country_code", ap_info.country.cc);
        cJSON_AddNumberToObject(json_wifi_ap_country_info, "start_channel", ap_info.country.schan);
        cJSON_AddNumberToObject(json_wifi_ap_country_info, "total_channel_num", ap_info.country.nchan);
        cJSON_AddNumberToObject(json_wifi_ap_country_info, "max_tx_power", ap_info.country.max_tx_power);
        switch(country.policy) {
            CASE_STRING_MACRO(json_wifi_ap_country_info, "country_policy", WIFI_COUNTRY_POLICY_AUTO)
            CASE_STRING_MACRO(json_wifi_ap_country_info, "country_policy", WIFI_COUNTRY_POLICY_MANUAL)
        }
        cJSON_AddItemToObject(json_wifi_ap_info, "country", json_wifi_ap_country_info);

        cJSON_AddItemToObject(json_root, "connected_ap", json_wifi_ap_info);
    }

    // build ip address info
    cJSON *json_wifi_ip = cJSON_CreateObject();
    if(!LDM::WiFi::connected) {
        ESP_LOGE(WIFI_TAG, "%s IP Fetch failed: %s\n", __func__, esp_err_to_name(err));\
        cJSON_AddStringToObject(json_root, "ip_address", "NONE");
    } else {
        char ip[16];
        snprintf(ip, 16, IPSTR, IP2STR(&LDM::WiFi::ipv4_address));
        cJSON_AddStringToObject(json_wifi_ip, "ipv4", ip);
        // cJSON_AddStringToObject(json_wifi_ip, "ipv6", (char*)IP2STR(&ip_info->ip));
        snprintf(ip, 16, IPSTR, IP2STR(&LDM::WiFi::netmask));
        cJSON_AddStringToObject(json_wifi_ip, "netmask", ip);
        snprintf(ip, 16, IPSTR, IP2STR(&LDM::WiFi::gateway));
        cJSON_AddStringToObject(json_wifi_ip, "gateway", ip);
        cJSON_AddItemToObject(json_root, "connected_ap", json_wifi_ip);
    }
    // esp_netif_ip_info_t *ip_info = NULL;
    // err = esp_netif_get_ip_info(this->netif_sta, ip_info);
    // if(err != ESP_OK) {
    //     ESP_LOGE(WIFI_TAG, "%s IP Fetch failed: %s\n", __func__, esp_err_to_name(err));
    //     cJSON_AddStringToObject(json_root, "ip_address", "NONE");
    // } else {
    //     char ip[16];
    //     snprintf(ip, 16, IPSTR, IP2STR(&ip_info->ip));
    //     cJSON_AddStringToObject(json_wifi_ip, "ipv4", ip);
    //     // cJSON_AddStringToObject(json_wifi_ip, "ipv6", (char*)IP2STR(&ip_info->ip));
    //     snprintf(ip, 16, IPSTR, IP2STR(&ip_info->netmask));
    //     cJSON_AddStringToObject(json_wifi_ip, "netmask", ip);
    //     snprintf(ip, 16, IPSTR, IP2STR(&ip_info->gw));
    //     cJSON_AddStringToObject(json_wifi_ip, "gateway", ip);
    //     cJSON_AddItemToObject(json_root, "connected_ap", json_wifi_ip);
    // }

    // char* output = cJSON_Print(json_root);
    // printf("%s", output);
    // free(output);

    return json_root;
}
