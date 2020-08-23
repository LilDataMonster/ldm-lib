#include <esp_log.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_blufi_api.h>

#include <wifi.hpp>
#include <bluetooth.hpp>
#include <ble.hpp>

#define TAG "LDM:BLE"

#define ERR_CHECK(_x, _msg) \
if(_x != ESP_OK) {\
    ESP_LOGE(TAG, "%s "#_msg": %s\n", __func__, esp_err_to_name(err));\
    return err;\
}

// static defines
LDM::WiFi LDM::BLE::wifi;

// Bluetooth in BLE Mode
LDM::BLE::BLE(char* device_name) : Bluetooth(device_name) {
    // set device name
    LDM::BLE::device_name = device_name;
    LDM::BLE::connected = false;
}

esp_err_t LDM::BLE::init(void) {
    // initialize bluetooth in BLE mode
    // esp_err_t err = bluetooth.init(ESP_BT_MODE_BLE);
    esp_err_t err = LDM::Bluetooth::init(ESP_BT_MODE_BLE);
    ERR_CHECK(err, "Error initializing bluetooth");

    err |= esp_ble_gap_set_device_name(this->device_name);
    ERR_CHECK(err, "Error setting BLE device name");

    return err;
}

esp_err_t LDM::BLE::deinit(void) {
    esp_err_t err = LDM::Bluetooth::deinit();
    ERR_CHECK(err, "Error deinitializing bluetooth");
    return err;
}

// GAP
esp_err_t LDM::BLE::registerGapCallback(esp_gap_ble_cb_t callback) {
    esp_err_t err = esp_ble_gap_register_callback(callback);
    ERR_CHECK(err, "Error BLE GAP register callback failed");
    return err;
}

esp_err_t LDM::BLE::configGapAdvData(esp_ble_adv_data_t *adv_data) {
    esp_err_t err = esp_ble_gap_config_adv_data(adv_data);
    ERR_CHECK(err, "Error BLE GAP ADV configuration failed");
    return ESP_OK;
}

esp_err_t LDM::BLE::startGapScan(uint32_t duration) {
    esp_err_t err = esp_ble_gap_start_scanning(duration);
    ERR_CHECK(err, "Error BLE GAP start scan failed");
    return err;
}

esp_err_t LDM::BLE::stopGapScan(void) {
    esp_err_t err = esp_ble_gap_stop_scanning();
    ERR_CHECK(err, "Error BLE GAP stop scan failed");
    return err;
}

// GATT Server
esp_err_t LDM::BLE::registerGattServerCallback(esp_gatts_cb_t callback) {
    esp_err_t err = esp_ble_gatts_register_callback(callback);
    ERR_CHECK(err, "Error BLE GATTS register callback failed");
    return err;
}

esp_err_t LDM::BLE::registerGattServerAppId(uint16_t app_id) {
    esp_err_t err = esp_ble_gatts_app_register(app_id);
    ERR_CHECK(err, "Error LE GATTS register app failed");
    return err;
}

esp_err_t LDM::BLE::unregisterGattServerApp(esp_gatt_if_t gatts_if) {
    esp_err_t err = esp_ble_gatts_app_unregister(gatts_if);
    ERR_CHECK(err, "Error BLE GATTS unregister app failed");
    return err;
}

// BluFi
esp_err_t LDM::BLE::registerBlufiCallback(esp_blufi_callbacks_t *callbacks) {
    esp_err_t err = esp_blufi_register_callbacks(callbacks);
    ERR_CHECK(err, "Error BLE BluFi register callback failed");
    return err;
}

esp_err_t LDM::BLE::initBlufi(void) {
    esp_err_t err = esp_blufi_profile_init();
    ERR_CHECK(err, "Error BLE BluFi initialization failed");

    err |= LDM::BLE::wifi.init();
    ERR_CHECK(err, "Error BLE BluFi initialization failed");

    ESP_LOGI(TAG, "BluFi Initialized");
    return err;
}

esp_err_t LDM::BLE::deinitBlufi(void) {
    esp_err_t err = esp_blufi_profile_deinit();
    ERR_CHECK(err, "Error BLE BluFi initialization failed");

    err |= LDM::BLE::wifi.deinit();
    ERR_CHECK(err, "Error BLE BluFi initialization failed");

    ESP_LOGI(TAG, "BluFi Deinitialized");
    return err;
}

uint16_t LDM::BLE::getBlufiVersion(void) {
    return esp_blufi_get_version();
}

bool LDM::BLE::isConnected(void) {
    return LDM::BLE::connected;
}
