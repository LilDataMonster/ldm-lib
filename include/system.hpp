#ifndef __SYS_HPP__
#define __SYS_HPP__

#include <esp_system.h>
#include <esp_app_format.h>
#include <cJSON.h>

namespace LDM {
class System {
public:
    System(uint8_t *custom_mac_address=NULL);

    // esp_err_t buildData(void);
    // float getHumidity(void);
    // float getTemperature(void);
    // void setHumidity(float humidity);
    // void setTemperature(float temperature);
    //
    // esp_err_t init(void);
    // esp_err_t deinit(void);
    // esp_err_t readSensor(void);
    cJSON *buildJson(void);

private:
    uint8_t base_mac_address[6];
    esp_chip_info_t chip_info;
    const char* idf_version;
    esp_reset_reason_t reset_reason;
    const esp_app_desc_t *app_info;
    const esp_partition_t *partition;
    // float temperature;
    // float humidity;
    //
    // static const gpio_num_t dht_gpio = static_cast<gpio_num_t>(DHT_GPIO);
    // static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
};
}
#endif
