#include <esp_sleep.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <mdns.hpp>

#define TAG "LDM-LIB:MDNS"

#define MDNS_HOSTNAME CONFIG_MDNS_HOSTNAME
#define MDNS_INSTANCE CONFIG_MDNS_INSTANCE

LDM::MDNS::MDNS() {
}

LDM::MDNS::~MDNS() {
}

esp_err_t LDM::MDNS::init(void) {
    esp_err_t err = mdns_init();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS Failed to Initialize");
        return err;
    }

    // set hostname
    mdns_hostname_set(MDNS_HOSTNAME);
    // set default instance
    mdns_instance_name_set(MDNS_INSTANCE);

    return mdns_init();
}

esp_err_t LDM::MDNS::deinit(void) {
    mdns_free();
    return ESP_OK;
}
