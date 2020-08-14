#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <nvs_flash.h>

namespace LDM {
class NVS {
public:
    NVS(nvs_handle_t nvs_h=-1);
    esp_err_t initDefault(void);

    // NVS Setup
    esp_err_t openNamespace(const char* ns, nvs_open_mode_t mode=NVS_READWRITE);
    esp_err_t commit(void);
    void close(void);

    // could probably use templates for this
    esp_err_t setKeyU8(const char *key, uint8_t value);
    esp_err_t getKeyU8(const char *key, uint8_t *value);
    esp_err_t setKeyU16(const char *key, uint16_t value);
    esp_err_t getKeyU16(const char *key, uint16_t *value);
    esp_err_t setKeyU32(const char *key, uint32_t value);
    esp_err_t getKeyU32(const char *key, uint32_t *value);
    esp_err_t setKeyU64(const char *key, uint64_t value);
    esp_err_t getKeyU64(const char *key, uint64_t *value);

    esp_err_t setKeyI8(const char *key, int8_t value);
    esp_err_t getKeyI8(const char *key, int8_t *value);
    esp_err_t setKeyI16(const char *key, int16_t value);
    esp_err_t getKeyI16(const char *key, int16_t *value);
    esp_err_t setKeyI32(const char *key, int32_t value);
    esp_err_t getKeyI32(const char *key, int32_t *value);
    esp_err_t setKeyI64(const char *key, int64_t value);
    esp_err_t getKeyI64(const char *key, int64_t *value);

    esp_err_t setKeyStr(const char *key, char *value);
    esp_err_t getKeyStr(const char *key, char *value, size_t *size);
    esp_err_t setKeyBlob(const char *key, char *value, size_t size);
    esp_err_t getKeyBlob(const char *key, void *value, size_t *size);

    esp_err_t eraseNVSKey(const char *key);
    esp_err_t eraseNVSAllKeys(void);

    esp_err_t getStats(const char *part_name, nvs_stats_t *nvs_stats);

private:
    esp_err_t setupNVS(void);

    nvs_handle_t nvs_h;
};
}

#endif
