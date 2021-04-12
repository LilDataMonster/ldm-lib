#ifndef __BLE_HPP__
#define __BLE_HPP__

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_blufi_api.h>

#include <wifi.hpp>
#include <bluetooth.hpp>

namespace LDM {
class BLE : public Bluetooth {
public:
    BLE(char* device_name);
    ~BLE();

    esp_err_t init(void);
    esp_err_t deinit(void);

    // GAP
    esp_err_t registerGapCallback(esp_gap_ble_cb_t callback);
    esp_err_t configGapAdvData(esp_ble_adv_data_t *adv_data);
    esp_err_t startGapScan(uint32_t duration);
    esp_err_t stopGapScan(void);

    // GATT Server
    esp_err_t registerGattServerCallback(esp_gatts_cb_t callback);
    esp_err_t registerGattServerAppId(uint16_t app_id);
    esp_err_t unregisterGattServerApp(esp_gatt_if_t gatts_if);

    // BluFi
    esp_err_t registerBlufiCallback(esp_blufi_callbacks_t *callbacks);
    esp_err_t initBlufi(void);
    esp_err_t initBlufi(wifi_config_t *wifi_config);
    esp_err_t deinitBlufi(void);
    uint16_t getBlufiVersion(void);
    static void defaultGapHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    static void defaultBlufiCallback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
    esp_err_t setupDefaultBleGapCallback(void);
    esp_err_t setupDefaultBlufiCallback(void);
    bool isConnected(void);

    static esp_ble_adv_data_t default_ble_adv_data;
    static esp_ble_adv_params_t default_ble_adv_params;
    LDM::WiFi wifi;

private:

    // std::string device_name;
    char* device_name;
    bool connected;

    // BluFi profile default callback info
    static wifi_config_t sta_config;
    static wifi_config_t ap_config;
    static uint8_t server_if;
    static uint16_t conn_id;

    // BluFi profile: store the station info for send back to phone
    static bool gl_sta_connected;
    static bool ble_is_connected;
    static uint8_t gl_sta_bssid[6];
    static uint8_t gl_sta_ssid[32];
    static int gl_sta_ssid_len;
};
}

#endif
