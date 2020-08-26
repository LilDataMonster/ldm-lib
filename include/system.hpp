#ifndef __SYS_HPP__
#define __SYS_HPP__

#include <esp_system.h>
#include <esp_partition.h>
#include <esp_app_format.h>
#include <cJSON.h>

namespace LDM {
class System {
public:
    System(uint8_t *custom_mac_address=NULL);
    cJSON *buildJson(void);

private:
    uint8_t base_mac_address[6];
    esp_chip_info_t chip_info;
    const char* idf_version;
    esp_reset_reason_t reset_reason;
    const esp_app_desc_t *app_info;
    const esp_partition_t *partition;
};
}
#endif
