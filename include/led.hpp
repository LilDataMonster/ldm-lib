#ifndef __LED_HPP__
#define __LED_HPP__

#include <driver/ledc.h>
#include <vector>

#define LEDC_TEST_DUTY 4000
#define LEDC_TEST_FADE_TIME 3000

namespace LDM {
class LED {
public:
    // LED();
    ~LED();

    esp_err_t init(void);
    esp_err_t deinit(void);
    esp_err_t configLedTimer(ledc_timer_config_t config);
    esp_err_t addLedChannelConfig(ledc_channel_config_t config);
    esp_err_t fadeLedWithTime(uint16_t channel_idx, uint32_t target_duty=LEDC_TEST_DUTY,
        int32_t max_fade_time_ms=LEDC_TEST_FADE_TIME, ledc_fade_mode_t fade_mode=LEDC_FADE_NO_WAIT);
    esp_err_t fadeLedWithStep(uint16_t channel_idx, uint32_t target_duty,
        int32_t scale, int32_t cycle_num, ledc_fade_mode_t fade_mode=LEDC_FADE_NO_WAIT);
    // esp_err_t fadeLedWithTime(uint16_t channel_idx, uint32_t target_duty, int32_t max_fade_time_ms, ledc_fade_mode_t fade_mode);
    esp_err_t setDuty(uint16_t channel_idx, uint32_t target_duty);

private:
    // ledc configuration timers
    ledc_timer_config_t ledc_timer;

    // individual configuration for each channel of LED controller
    std::vector<ledc_channel_config_t> ledc_channels;
};
}

#endif // __LED_HPP__
