#include <esp_sleep.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <sys/time.h>

#include <nvs.hpp>

#define TAG "LDM-NVS"

RTC_DATA_ATTR struct timeval LDM::NVS::sleep_enter_time;

LDM::NVS::NVS(nvs_handle_t nvs_h) {
    if(nvs_h != -1) {
        // setup with custom provided initial setup
        this->nvs_h = nvs_h;
    } else {
        // setup default initial setup
        esp_err_t ret = initDefault();
        ESP_ERROR_CHECK(ret);
    }
}

esp_err_t LDM::NVS::initDefault(void) {
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    return ret;
}

esp_sleep_source_t LDM::NVS::getWakeupCause(void) {

    struct timeval now;
    gettimeofday(&now, NULL);
    const int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    esp_sleep_source_t cause = esp_sleep_get_wakeup_cause();
    switch(cause) {
        case ESP_SLEEP_WAKEUP_TIMER: {
            ESP_LOGI(TAG, "Wake up from timer. Time spent in deep sleep: %dms", sleep_time_ms);
            break;
        }
        case ESP_SLEEP_WAKEUP_ULP: {
            ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_ULP");
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED: {
            ESP_LOGI(TAG, "Wakeup was not caused by deep sleep");
            break;
        }
        default:
            ESP_LOGI(TAG, "Not a deep sleep reset");
    }
    return cause;
}

esp_err_t LDM::NVS::openNamespace(const char* ns, nvs_open_mode_t mode) {
    esp_err_t ret = nvs_open(ns, mode, &this->nvs_h);

    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t LDM::NVS::commit(void) {
    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.
    ESP_LOGI(TAG, "Committing updates in NVS ... ");
    esp_err_t ret = nvs_commit(nvs_h);

    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) commiting broadcast state", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Committed NVS Updates");
    }
    return ret;
}

void LDM::NVS::close(void) {
    nvs_close(this->nvs_h);
}

esp_err_t LDM::NVS::getKeyU8(const char *key, uint8_t *value) {
    esp_err_t ret = nvs_get_u8(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %u", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyU8(const char *key, uint8_t value) {
    esp_err_t ret = nvs_set_u8(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %u", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyU16(const char *key, uint16_t *value) {
    esp_err_t ret = nvs_get_u16(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %u", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyU16(const char *key, uint16_t value) {
    esp_err_t ret = nvs_set_u16(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %u", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyU32(const char *key, uint32_t *value) {
    esp_err_t ret = nvs_get_u32(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %u", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyU32(const char *key, uint32_t value) {
    esp_err_t ret = nvs_set_u32(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %u", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyU64(const char *key, uint64_t *value) {
    esp_err_t ret = nvs_get_u64(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
             ESP_LOGI(TAG, "NVS Get Key %s = %llu", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyU64(const char *key, uint64_t value) {
    esp_err_t ret = nvs_set_u64(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %llu", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyI8(const char *key, int8_t *value) {
    esp_err_t ret = nvs_get_i8(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %d", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyI8(const char *key, int8_t value) {
    esp_err_t ret = nvs_set_i8(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %d", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyI16(const char *key, int16_t *value) {
    esp_err_t ret = nvs_get_i16(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %d", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyI16(const char *key, int16_t value) {
    esp_err_t ret = nvs_set_i16(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %d", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyI32(const char *key, int32_t *value) {
    esp_err_t ret = nvs_get_i32(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %d", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyI32(const char *key, int32_t value) {
    esp_err_t ret = nvs_set_i32(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %d", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyI64(const char *key, int64_t *value) {
    esp_err_t ret = nvs_get_i64(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %lld", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyI64(const char *key, int64_t value) {
    esp_err_t ret = nvs_set_i64(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %lld", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyStr(const char *key, char *value, size_t *size) {
    esp_err_t ret = nvs_get_str(nvs_h, key, value, size);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s = %s", key, value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyStr(const char *key, char *value) {
    esp_err_t ret = nvs_set_str(nvs_h, key, value);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s = %s", key, value);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getKeyBlob(const char *key, void *value, size_t *size) {
    esp_err_t ret = nvs_get_blob(nvs_h, key, value, size);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Get Key %s Blob", key);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::setKeyBlob(const char *key, char *value, size_t size) {
    esp_err_t ret = nvs_set_blob(nvs_h, key, value, size);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s Blob", key);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE(TAG, "NVS Key Error: The key does not satisfy constraints");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE(TAG, "NVS Key Error: The length is not sufficient to store data");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

// nvs erase key
esp_err_t LDM::NVS::eraseNVSKey(const char *key) {
    esp_err_t ret = nvs_erase_key(nvs_h, key);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Set Key %s erased", key);
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Key Error: The key does not exist");
            break;
        case ESP_ERR_NVS_READ_ONLY:
            ESP_LOGE(TAG, "NVS Key Error: The key is opened as read-only");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::eraseNVSAllKeys(void) {
    esp_err_t ret = nvs_erase_all(nvs_h);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Keys Erased");
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE(TAG, "NVS Key Error: The NVS handle has been closed or is NULL");
            break;
        case ESP_ERR_NVS_READ_ONLY:
            ESP_LOGE(TAG, "NVS Key Error: The key is opened as read-only");
            break;
        default :
            ESP_LOGE(TAG, "NVS Key Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t LDM::NVS::getStats(const char *part_name, nvs_stats_t *nvs_stats) {
    esp_err_t ret = nvs_get_stats(part_name, nvs_stats);
    switch(ret) {
        case ESP_OK:
            ESP_LOGI(TAG, "NVS Partition (%s) Stats: Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)",
                part_name, nvs_stats->used_entries, nvs_stats->free_entries, nvs_stats->total_entries);
            break;
        case ESP_ERR_NVS_PART_NOT_FOUND:
            ESP_LOGE(TAG, "NVS Stats: The NVS partition (%s) cannot be found", part_name);
            break;
        case ESP_ERR_NVS_NOT_INITIALIZED:
            ESP_LOGE(TAG, "NVS Stats: The NVS partition (%s) storage is not initialized", part_name);
            break;
        case ESP_ERR_INVALID_ARG:
            ESP_LOGE(TAG, "NVS Stats: nvs_stats is NULL");
            break;
        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(TAG, "NVS Stats: Page status is INVALID");
            break;
        default:
            ESP_LOGE(TAG, "NVS Stats Error: (%s)", esp_err_to_name(ret));
    }
    return ret;
}
