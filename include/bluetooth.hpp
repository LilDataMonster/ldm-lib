#ifndef __BLUETOOTH_HPP__
#define __BLUETOOTH_HPP__

#include <esp_bt.h>

namespace LDM {
class Bluetooth {
public:
    Bluetooth(char* device_name);
    ~Bluetooth();

    esp_err_t init(esp_bt_mode_t bt_mode=ESP_BT_MODE_BLE);
    esp_err_t deinit(void);

    esp_bt_controller_status_t getStatus(void);
    esp_power_level_t getTxPowerLevel(esp_ble_power_type_t power_type);
    esp_err_t setTxPowerLevel(esp_ble_power_type_t power_type, esp_power_level_t power_level);

    // bluetooth sleep
    esp_err_t enableModemSleep(void);
    esp_err_t disableModemSleep(void);
    bool isSleeping(void);
    void requestWakeup(void);
    const uint8_t *getDeviceAddress(void);

     esp_err_t flushScanList(void);

    // TODO: add bluetooth audio
    //esp_bredr_sco_datapath_set

private:

    esp_err_t initializeController(esp_bt_mode_t bt_mode=ESP_BT_MODE_BLE);
    esp_err_t initializeBluedroid(void);
    esp_err_t deinitializeController(void);
    esp_err_t deinitializeBluedroid(void);

    esp_bt_controller_config_t config;
    esp_bt_mode_t bt_mode;

    bool controller_initialized;
    char* device_name;
};
}
#endif
