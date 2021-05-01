#ifndef __HTTP_HPP__
#define __HTTP_HPP__

// HTTP request
#include <esp_tls.h>
#include <esp_http_client.h>

// JSON formatting
#include <cJSON.h>

#include <string>

#include <http_content.hpp>

#define MAX_HTTP_OUTPUT_BUFFER 2048

namespace LDM {
class HTTP_Client {
public:
    HTTP_Client(char* URL);
    ~HTTP_Client(void);
    esp_err_t deinit(void);
    esp_http_client_handle_t getClient(void);
    const char * getURL(void);
    std::string getURLString(void);
    esp_err_t setURL(char* URL);
    esp_err_t postJSON(cJSON *message, size_t size=0);
    esp_err_t postFormattedJSON(char *message);
    esp_err_t postBuffer(const uint8_t *buffer, int32_t buffer_len, char* content_type=NULL);

private:
    std::string URL;
    char response_buffer[MAX_HTTP_OUTPUT_BUFFER];
    esp_http_client_config_t config;
    esp_http_client_handle_t client;
};
}
#endif
