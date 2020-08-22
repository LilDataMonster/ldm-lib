#ifndef __BLE_HPP__
#define __BLE_HPP__

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_blufi_api.h>

#include <bluetooth.hpp>

namespace LDM {
class BLE : public Bluetooth {
public:
    BLE(char* device_name);
    esp_err_t init(void);
    esp_err_t deinit(void);

    // GAP
    static esp_err_t registerGapCallback(esp_gap_ble_cb_t callback);
    static esp_err_t configGapAdvData(esp_ble_adv_data_t *adv_data);
    static esp_err_t startGapScan(uint32_t duration);
    static esp_err_t stopGapScan(void);

    // GATT Server
    static esp_err_t registerGattServerCallback(esp_gatts_cb_t callback);
    static esp_err_t registerGattServerAppId(uint16_t app_id);
    static esp_err_t unregisterGattServerApp(esp_gatt_if_t gatts_if);

    // BluFi
    static esp_err_t registerBlufiCallback(esp_blufi_callbacks_t *callbacks);
    static esp_err_t initBlufi(void);
    static esp_err_t deinitBlufi(void);
    static uint16_t getBlufiVersion(void);
    static void defaultGapHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    static void defaultBlufiCallback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
    static esp_err_t setupDefaultBlufiCallback(void);
    static bool isConnected(void);

    static esp_ble_adv_data_t default_blufi_adv_data;
    static esp_ble_adv_params_t default_blufi_adv_params;
    
private:

    // static std::string device_name;
    static char* device_name;
    static bool connected;

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
    // static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    // static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    // static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

    // static std::string device_name;
    // static uint8_t manufacturer_data[MANUFACTURER_DATA_LEN];
    //
    // static uint8_t adv_service_uuid128[32] ;
    // static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM];
    //
    // static esp_ble_adv_data_t adv_data;
    // static esp_ble_adv_data_t scan_rsp_data;
    //
    // static uint8_t adv_config_done;
};
}

#endif
