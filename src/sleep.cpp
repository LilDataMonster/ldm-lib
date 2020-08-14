#include <esp_sleep.h>
#include <esp_log.h>
#include <sys/time.h>

#include <sleep.hpp>

#define TAG "LDM-SLEEP"

RTC_DATA_ATTR struct timeval LDM::Sleep::sleep_enter_time;

LDM::Sleep::Sleep(void) {

}

esp_sleep_source_t LDM::Sleep::getWakeupCause(void) {

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

void LDM::Sleep::enterDeepSleepUsec(uint64_t timeMicrosec) {
    // Call to this function is equivalent to a call to
    // esp_deep_sleep_enable_timer_wakeup followed
    // by a call to esp_deep_sleep_start.
    ESP_LOGI(TAG, "Entering Deep Sleep for %llu microseconds", timeMicrosec);
    esp_deep_sleep(timeMicrosec);
}

void LDM::Sleep::enterDeepSleepSec(uint64_t timeSec) {
    LDM::Sleep::enterDeepSleepUsec(timeSec * 1E6);
}
