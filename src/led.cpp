#include <esp_log.h>
#include <driver/ledc.h>

#include <led.hpp>

#define TAG "LDM:LED"

#define ERR_CHECK(_x, _msg) \
if(_x != ESP_OK) {\
    ESP_LOGE(TAG, "%s "#_msg": %s\n", __func__, esp_err_to_name(err));\
    return err;\
}

// LDM::LED::LED() {
//     //
// }

LDM::LED::~LED() {
    this->deinit();
}

esp_err_t LDM::LED::init(void) {
    esp_err_t err = configLedTimer(this->ledc_timer);
    ERR_CHECK(err, "Invalid LED timer configuration");

    // set LED controller with previously prepared configuration
    for(auto channel : this->ledc_channels) {
        err = ledc_channel_config(&channel);
        ERR_CHECK(err, "Invalid LED channel configuration");
    }

    // initialize fade service
    err = ledc_fade_func_install(0);
    ERR_CHECK(err, "LED Fade Fuction already initialized");

    return err;
}

esp_err_t LDM::LED::deinit(void) {
    ledc_fade_func_uninstall();
    ledc_channels.clear();

    return ESP_OK;
}

esp_err_t LDM::LED::configLedTimer(ledc_timer_config_t config) {
    this->ledc_timer = config;
    esp_err_t err = ledc_timer_config(&this->ledc_timer);
    ERR_CHECK(err, "Invalid LED timer configuration");
    return err;
}

esp_err_t LDM::LED::addLedChannelConfig(ledc_channel_config_t config) {
    ledc_channels.push_back(config);
    return ESP_OK;
}

esp_err_t LDM::LED::fadeLedWithTime(uint16_t channel_idx, uint32_t target_duty, int32_t max_fade_time_ms, ledc_fade_mode_t fade_mode) {

    esp_err_t err = ESP_OK;
    err = ledc_set_fade_with_time(ledc_channels[channel_idx].speed_mode,
              ledc_channels[channel_idx].channel, target_duty, max_fade_time_ms);
    ERR_CHECK(err, "LED setting fade failed");

    err = ledc_fade_start(ledc_channels[channel_idx].speed_mode,
              ledc_channels[channel_idx].channel, fade_mode);
    ERR_CHECK(err, "LED starting fade failed");

    return err;
}

esp_err_t LDM::LED::fadeLedWithStep(uint16_t channel_idx, uint32_t target_duty, int32_t scale, int32_t cycle_num, ledc_fade_mode_t fade_mode) {

    esp_err_t err = ESP_OK;
    err = ledc_set_fade_with_step(ledc_channels[channel_idx].speed_mode,
              ledc_channels[channel_idx].channel, target_duty, scale, cycle_num);
    ERR_CHECK(err, "LED setting fade failed");

    err = ledc_fade_start(ledc_channels[channel_idx].speed_mode,
              ledc_channels[channel_idx].channel, fade_mode);
    ERR_CHECK(err, "LED starting fade failed");

    return err;
}

esp_err_t LDM::LED::setDuty(uint16_t channel_idx, uint32_t target_duty) {

    esp_err_t err = ESP_OK;
    err = ledc_set_duty(ledc_channels[channel_idx].speed_mode, ledc_channels[channel_idx].channel, target_duty);
    ERR_CHECK(err, "LED setting duty failed");

    err = ledc_update_duty(ledc_channels[channel_idx].speed_mode, ledc_channels[channel_idx].channel);
    ERR_CHECK(err, "LED updating duty failed");

    return err;
}
