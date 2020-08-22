#include <string>

#include <esp_log.h>

#include <esp_bt.h>
#include <esp_bt_device.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_main.h>

#include <bluetooth.hpp>

#define TAG BLUETOOTH_TAG

#define ERR_CHECK(_x, _msg) \
if(_x != ESP_OK) {\
    ESP_LOGE(TAG, "%s "#_msg": %s\n", __func__, esp_err_to_name(err));\
    return err;\
}

LDM::Bluetooth::Bluetooth(char* device_name) {
    this->device_name = device_name;

    // clear out bluetooth controller/release Bluetooth classic
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // setup default bluetooth configuration
    this->config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
}

LDM::Bluetooth::~Bluetooth() {
    //
    this->deinit();
}

esp_err_t LDM::Bluetooth::init(esp_bt_mode_t bt_mode) {
    esp_err_t err = ESP_OK;
    err |= this->initializeController();
    ERR_CHECK(err, "Init bluetooth failed");

    err |= this->initializeBluedroid();
    ERR_CHECK(err, "Init bluetooth failed");

    err |= esp_bt_dev_set_device_name(this->device_name);
    ERR_CHECK(err, "Init bluetooth setting device name failed");

    ESP_LOGI(TAG, "Bluetooth Initialized");
    this->controller_initialized = true;
    return err;
}

esp_err_t LDM::Bluetooth::deinit(void) {
    esp_err_t err = ESP_OK;
    err |= this->deinitializeBluedroid();
    ERR_CHECK(err, "Deinit bluetooth failed");

    err |= this->deinitializeController();
    ERR_CHECK(err, "Deinit bluetooth failed");

    ESP_LOGI(TAG, "Bluetooth Deinitialized");
    this->controller_initialized = false;
    return err;
}

esp_err_t LDM::Bluetooth::initializeController(esp_bt_mode_t bt_mode) {
    this->bt_mode = bt_mode;

    // initialize conroller
    esp_err_t err = esp_bt_controller_init(&this->config);
    ERR_CHECK(err, "Init bluetooth controller failed");

    // enable controller
    err |= esp_bt_controller_enable(this->bt_mode);
    ERR_CHECK(err, "Enable bluetooth controller failed");

    this->controller_initialized = true;
    return err;
}

esp_err_t LDM::Bluetooth::initializeBluedroid(void) {
    // initialize bluedroid
    esp_err_t err = esp_bluedroid_init();
    ERR_CHECK(err, "Init bluedroid failed");

    // enable bluedroid
    err |= esp_bluedroid_enable();
    ERR_CHECK(err, "Enable bluedroid failed");

    return err;
}

esp_err_t LDM::Bluetooth::deinitializeController(void) {
    // disable controller
    esp_err_t err = esp_bt_controller_disable();
    ERR_CHECK(err, "Disable bluetooth controller failed");

    // deinit controller
    err |= esp_bt_controller_deinit();
    ERR_CHECK(err, "Deinit bluetooth controller failed");

    // free bluetooth stack memory
    err |= esp_bt_mem_release(this->bt_mode);
    ERR_CHECK(err, "Deinit bluetooth controller failed");

    return err;
}

esp_err_t LDM::Bluetooth::deinitializeBluedroid(void) {
    // disable bluedroid
    esp_err_t err = esp_bluedroid_disable();
    ERR_CHECK(err, "Disable bluedroid failed");

    // deinit bluedroid
    err |= esp_bluedroid_deinit();
    ERR_CHECK(err, "Deinit bluedroid failed");

    return err;
}

esp_bt_controller_status_t LDM::Bluetooth::getStatus(void) {
    return esp_bt_controller_get_status();
}

esp_power_level_t LDM::Bluetooth::getTxPowerLevel(esp_ble_power_type_t power_type) {
    // TODO: Add error checks
    return esp_ble_tx_power_get(power_type);
}

esp_err_t setTxPowerLevel(esp_ble_power_type_t power_type, esp_power_level_t power_level) {
    // TODO: Add error checks
    esp_err_t err = esp_ble_tx_power_set(power_type, power_level);
    ERR_CHECK(err, "Setting Tx Power Level failed");

    return err;
}

esp_err_t LDM::Bluetooth::enableModemSleep(void) {
    if(this->controller_initialized) {
        return esp_bt_sleep_enable();
    } else {
        ESP_LOGE(TAG, "%s Bluetooth sleep enable error! Bluetooth controller is not enabled\n", __func__);
        return ESP_FAIL;
    }
}

esp_err_t LDM::Bluetooth::disableModemSleep(void) {
    if(this->controller_initialized) {
        return esp_bt_sleep_disable();
    } else {
        ESP_LOGE(TAG, "%s Bluetooth sleep disable error! Bluetooth controller is not enabled\n", __func__);
        return ESP_FAIL;
    }
}

bool LDM::Bluetooth::isSleeping(void) {
    if(this->controller_initialized) {
        return esp_bt_controller_is_sleeping();
    } else {
        ESP_LOGE(TAG, "%s Bluetooth sleep check error! Bluetooth controller is not enabled\n", __func__);
        return ESP_FAIL;
    }
}

void LDM::Bluetooth::requestWakeup(void) {
    if(this->controller_initialized) {
        esp_bt_controller_wakeup_request();
    } else {
        ESP_LOGE(TAG, "%s Bluetooth sleep check error! Bluetooth controller is not enabled\n", __func__);
    }
}

esp_err_t LDM::Bluetooth::flushScanList(void) {
    return esp_ble_scan_dupilcate_list_flush();
}

const uint8_t * LDM::Bluetooth::getDeviceAddress(void) {
    if(this->controller_initialized) {
        return esp_bt_dev_get_address();
    } else {
        return NULL;
    }
}
