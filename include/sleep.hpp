#ifndef __SLEEP_HPP__
#define __SLEEP_HPP__

#include <esp_sleep.h>

namespace LDM {
class Sleep {
public:
    Sleep(void);
    static esp_sleep_source_t getWakeupCause(void);

    // Deep Sleep
    /*
      Make sure relevant WiFi and BT stack functions are called to close any connections and deinitialize the peripherals. These include:
        esp_bluedroid_disable
        esp_bt_controller_disable
        esp_wifi_stop
    */
    static void enterDeepSleepUs(uint64_t timeMicrosec);
    static void enterDeepSleepS(uint64_t timeSec);


    // Light Sleep
    //TODO
    
private:
    // set RTC memory
    static RTC_DATA_ATTR struct timeval sleep_enter_time;

};
}
#endif
