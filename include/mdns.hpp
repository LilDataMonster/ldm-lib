#ifndef __MDNS_HPP__
#define __MDNS_HPP__

#include <mdns.h>

namespace LDM {
class MDNS {
public:
    MDNS();
    ~MDNS();

    esp_err_t init(void);
    esp_err_t deinit(void);
    // TODO: Add other mDNS capabilities
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mdns.html

private:

};
}

#endif // __MDNS_HPP__
