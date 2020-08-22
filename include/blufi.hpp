#ifndef __BLUFI_HPP__
#define __BLUFI_HPP__

// Bluetooth support
#include <esp_bt.h>

#include <esp_bt_defs.h>
#include <esp_gap_ble_api.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>

// Blufi support
#include <esp_blufi_api.h>

namespace LDM {
class BluFi {
public:
    BluFi();
    esp_err_t init(void);
    // esp_http_client_handle_t getClient(void) {
    //     return this->client;
    // }
    // esp_err_t postJSON(cJSON *message, size_t size=0);
    // esp_err_t postFormattedJSON(char *message);
    // esp_err_t deinit(void) {
    //     return esp_http_client_cleanup(this->client);
    // }

private:
    // char response_buffer[MAX_HTTP_OUTPUT_BUFFER];
    // esp_http_client_config_t config;
    // esp_http_client_handle_t client;
};
}
#endif
