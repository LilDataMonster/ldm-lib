#include <esp_sleep.h>
#include <esp_log.h>
#include <sys/time.h>

#include <sleep.hpp>

#define TAG "LDM-LIB:SLEEP"

RTC_DATA_ATTR struct timeval LDM::Sleep::sleep_enter_time;

LDM::Sleep::Sleep(void) {

}

esp_sleep_source_t LDM::Sleep::getWakeupCause(void) {

    struct timeval now;
    gettimeofday(&now, NULL);
    const int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    esp_sleep_source_t cause = esp_sleep_get_wakeup_cause();
    switch(cause) {
        case ESP_SLEEP_WAKEUP_UNDEFINED: {
            ESP_LOGI(TAG, "Wakeup was not caused by exit from deep sleep");
            break;
        }
        case ESP_SLEEP_WAKEUP_EXT0: {
            ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_IO");
            break;
        }
        case ESP_SLEEP_WAKEUP_EXT1: {
            ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_CNTL");
            break;
        }
        case ESP_SLEEP_WAKEUP_TIMER: {
            ESP_LOGI(TAG, "Wakeup caused by timer. Time spent in deep sleep: %dms", sleep_time_ms);
            break;
        }
        case ESP_SLEEP_WAKEUP_TOUCHPAD: {
            ESP_LOGI(TAG, "Wakeup caused by touchpad");
            break;
        }
        case ESP_SLEEP_WAKEUP_ULP: {
            ESP_LOGI(TAG, "Wakup caused by ULP program");
            break;
        }
        case ESP_SLEEP_WAKEUP_GPIO: {
            ESP_LOGI(TAG, "Wakeup caused by GPIO (light sleep only)");
            break;
        }
        case ESP_SLEEP_WAKEUP_UART: {
            ESP_LOGI(TAG, "Wakeup caused by UART (light sleep only)");
            break;
        }
        // case ESP_SLEEP_WAKEUP_WIFI: {
        //     ESP_LOGI(TAG, "Wakeup caused by WIFI (light sleep only)");
        //     break;
        // }
        // case ESP_SLEEP_WAKEUP_BT: {
        //     ESP_LOGI(TAG, "Wakeup caused by BT (light sleep only)");
        //     break;
        // }
        // case ESP_SLEEP_WAKEUP_COCPU: {
        //     ESP_LOGI(TAG, "Wakeup caused by COCPU int");
        //     break;
        // }
        // case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG: {
        //     ESP_LOGI(TAG, "Wakeup caused by COCPU crash");
        //     break;
        // }
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
