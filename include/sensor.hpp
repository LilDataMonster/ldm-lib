#ifndef __SENSOR_HPP__
#define __SENSOR_HPP__

#include <cJSON.h>

namespace LDM {
class Sensor {
public:
    // sensors must implement the following functions
    virtual esp_err_t init(void) = 0;
    virtual esp_err_t deinit(void) = 0;
    virtual esp_err_t readSensor(void) = 0;
    virtual cJSON *buildJson(void) = 0;
};
}

#endif
