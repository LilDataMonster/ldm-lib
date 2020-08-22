#include <esp_log.h>
#include <esp_system.h>
#include <esp_app_format.h>
#include <esp_ota_ops.h>
#include <cstring>

#include <system.hpp>

#define TAG "LDM-SYSTEM"

#define CASE_STRING_MACRO(_cjson, _name, _macro) case _macro: \
    cJSON_AddStringToObject(_cjson, _name, #_macro); \
    break;

LDM::System::System(uint8_t *custom_mac_address) {

    // define custom mac address
    if(custom_mac_address != NULL) {
        memcpy(this->base_mac_address, custom_mac_address, 6);
    } else  {
        // read custom mac address from BLK3
        esp_err_t ret = esp_efuse_mac_get_custom(this->base_mac_address);
        if(ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get MAC Address from EFUSE BLK3. (%s)", esp_err_to_name(ret));
            ESP_LOGI(TAG, "Defaulting to base MAC Address in BLK0 of EFUSE");

            // read default mac address from BLK0
            ret = esp_efuse_mac_get_default(this->base_mac_address);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to get base MAC Address from EFUSE BLK0. (%s)", esp_err_to_name(ret));
            } else {
                ESP_LOGI(TAG, "Base MAC Address read from EFUSE BLK0");
            }
        } else {
            ESP_LOGI(TAG, "Base MAC Address read from EFUSE BLK3");
        }
    }

    // set mac address
    esp_base_mac_addr_set(this->base_mac_address);
    ESP_LOGI(TAG, "Base MAC Address set: %02X:%02X:%02X:%02X:%02X:%02X",
             this->base_mac_address[0], this->base_mac_address[1], this->base_mac_address[2],
             this->base_mac_address[3], this->base_mac_address[4], this->base_mac_address[5]);

    // get chip info
    esp_chip_info(&this->chip_info);

    // get idf info
    this->idf_version = esp_get_idf_version();

    // get app info
    this->app_info = esp_ota_get_app_description();

    // get parition info
    this->partition = esp_ota_get_running_partition();

    // get reset info
    this->reset_reason = esp_reset_reason();
}


cJSON* LDM::System::buildJson(void) {
    cJSON *json_root = cJSON_CreateObject();

    // build mac address
    char address_string[18];
    uint8_t mac[6];

    cJSON *json_mac_address = cJSON_CreateObject();
    cJSON_AddItemToObject(json_root, "mac_address", json_mac_address);

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    snprintf(address_string, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(json_mac_address, "wifi_station", address_string);

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP));
    snprintf(address_string, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(json_mac_address, "wifi_softap", address_string);

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_BT));
    snprintf(address_string, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(json_mac_address, "bluetooth", address_string);

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_ETH));
    snprintf(address_string, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(json_mac_address, "ethernet", address_string);

    // build chip info
    cJSON *json_chip_info = cJSON_CreateObject();
    cJSON_AddItemToObject(json_root, "chip_info", json_chip_info);

    switch(this->chip_info.model) {
        CASE_STRING_MACRO(json_chip_info, "model", CHIP_ESP32)
        CASE_STRING_MACRO(json_chip_info, "model", CHIP_ESP32S2)
        CASE_STRING_MACRO(json_chip_info, "model", CHIP_ESP32S3)
    }
    cJSON *json_chip_features = cJSON_AddArrayToObject(json_chip_info, "chip_features");
    if(this->chip_info.features | CHIP_FEATURE_EMB_FLASH) {
        cJSON_AddItemToArray(json_chip_features, cJSON_CreateString("EMBEDDED_FLASH_MEM"));
    }
    if(this->chip_info.features | CHIP_FEATURE_WIFI_BGN) {
        cJSON_AddItemToArray(json_chip_features, cJSON_CreateString("2.4_GHZ_WIFI"));
    }
    if(this->chip_info.features | CHIP_FEATURE_BLE) {
        cJSON_AddItemToArray(json_chip_features, cJSON_CreateString("BLUETOOTH_LOW_ENERGY"));
    }
    if(this->chip_info.features | CHIP_FEATURE_BT) {
        cJSON_AddItemToArray(json_chip_features, cJSON_CreateString("BLUETOOTH_CLASSIC"));
    }
    cJSON_AddNumberToObject(json_chip_info, "num_cores", this->chip_info.cores);
    cJSON_AddNumberToObject(json_chip_info, "chip_revision_num", this->chip_info.revision);

    // build idf info
    // cJSON *json_idf_info = cJSON_CreateObject();
    cJSON_AddStringToObject(json_root, "idf_version", this->idf_version);

    // build heap size info
    cJSON *json_heap_info = cJSON_CreateObject();
    cJSON_AddItemToObject(json_root, "heap_info", json_heap_info);
    cJSON_AddNumberToObject(json_heap_info, "heap_size", esp_get_free_heap_size());
    cJSON_AddNumberToObject(json_heap_info, "internal_heap_size", esp_get_free_internal_heap_size());
    cJSON_AddNumberToObject(json_heap_info, "min_heap_size", esp_get_minimum_free_heap_size());

    // build reset reason info
    switch(this->reset_reason) {
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_UNKNOWN)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_POWERON)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_EXT)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_SW)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_PANIC)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_INT_WDT)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_TASK_WDT)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_WDT)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_DEEPSLEEP)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_BROWNOUT)
        CASE_STRING_MACRO(json_root, "reset_reason", ESP_RST_SDIO)
    }

    // build app info
    cJSON *json_app_info = cJSON_CreateObject();
    cJSON_AddItemToObject(json_root, "app_info", json_app_info);

    cJSON_AddNumberToObject(json_app_info, "magic_word", this->app_info->magic_word);
    cJSON_AddNumberToObject(json_app_info, "secure_version", this->app_info->secure_version);

    cJSON *json_reserv1 = cJSON_AddArrayToObject(json_app_info, "reserv1");
    for(int i=0; i < (sizeof(this->app_info->reserv1)/sizeof(*this->app_info->reserv1)); i++) {
        cJSON_AddItemToArray(json_reserv1, cJSON_CreateNumber(this->app_info->reserv1[i]));
    }

    cJSON *json_reserv2 = cJSON_AddArrayToObject(json_app_info, "reserv2");
    for(int i=0; i < (sizeof(this->app_info->reserv2)/sizeof(*this->app_info->reserv2)); i++) {
        cJSON_AddItemToArray(json_reserv2, cJSON_CreateNumber(this->app_info->reserv2[i]));
    }

    cJSON_AddStringToObject(json_app_info, "version", this->app_info->version);
    cJSON_AddStringToObject(json_app_info, "project_name", this->app_info->project_name);
    cJSON_AddStringToObject(json_app_info, "time", this->app_info->time);
    cJSON_AddStringToObject(json_app_info, "date", this->app_info->date);
    cJSON_AddStringToObject(json_app_info, "idf_ver", this->app_info->idf_ver);

    cJSON *json_app_elf = cJSON_AddArrayToObject(json_app_info, "app_elf_sha256");
    for(int i=0; i < (sizeof(this->app_info->app_elf_sha256)/sizeof(*this->app_info->app_elf_sha256)); i++) {
        cJSON_AddItemToArray(json_app_elf, cJSON_CreateNumber(this->app_info->app_elf_sha256[i]));
    }

    // build parition info
    cJSON *json_part_info = cJSON_CreateObject();
    cJSON_AddItemToObject(json_root, "partition_info", json_part_info);

    switch(this->partition->type) {
        CASE_STRING_MACRO(json_part_info, "type", ESP_PARTITION_TYPE_APP)
        CASE_STRING_MACRO(json_part_info, "type", ESP_PARTITION_TYPE_DATA)
    }

    switch(this->partition->subtype) {
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_FACTORY) // same as ESP_PARTITION_SUBTYPE_DATA_OTA
        // CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_MIN)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_0) // same as ESP_PARTITION_SUBTYPE_APP_OTA_MIN
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_1)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_2)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_3)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_4)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_5)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_6)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_7)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_8)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_9)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_10)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_11)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_12)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_13)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_14)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_15) // same as ESP_PARTITION_SUBTYPE_APP_OTA_MAX
        // CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_OTA_MAX)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_APP_TEST)
        // CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_OTA)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_PHY)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_NVS)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_COREDUMP)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_EFUSE_EM)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_FAT)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_DATA_SPIFFS)
        CASE_STRING_MACRO(json_part_info, "subtype", ESP_PARTITION_SUBTYPE_ANY)
    }

    cJSON_AddNumberToObject(json_part_info, "address", this->partition->address);
    cJSON_AddNumberToObject(json_part_info, "size", this->partition->size);
    cJSON_AddStringToObject(json_part_info, "label", this->partition->label);
    cJSON_AddItemToObject(json_part_info, "encrypted", cJSON_CreateBool(this->partition->encrypted));

    // char* output = cJSON_Print(json_root);
    // printf("%s", output);

    return json_root;
}

#undef CASE_STRING_MACRO
