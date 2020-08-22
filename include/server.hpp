#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <esp_http_server.h>
#include <vector>

// // JSON formatting
// #include <cJSON.h>
//
// #define MAX_HTTP_OUTPUT_BUFFER 2048

#define TAG "HTTP_SERVER"

#ifndef ESP_VFS_PATH_MAX
#define ESP_VFS_PATH_MAX 10
#endif

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

namespace LDM {
class Server {
public:
    Server(char* base_path);
    // Server();
    ~Server();

    // the WiFi stack must be initialized before starting the server
    esp_err_t startServer(void);
    esp_err_t stopServer(void);

    esp_err_t registerUriHandle(httpd_uri_t *uri);
    esp_err_t unregisterUriHandle(httpd_uri_t *uri);
    esp_err_t unregisterUriHandle(char* uri, httpd_method_t method);
    esp_err_t unregisterAllUriHandles(void);
    esp_err_t unregisterAllUriHandles(char* uri);

    // esp_http_client_handle_t getClient(void) {
    //     return this->client;
    // }
    // esp_err_t postJSON(cJSON *message, size_t size=0);
    // esp_err_t postFormattedJSON(char *message);
    // esp_err_t deinit(void) {
    //     return esp_http_client_cleanup(this->client);
    // }

private:
    httpd_handle_t server;
    httpd_config_t config;
    rest_server_context_t *context;

    std::vector<httpd_uri_t*> registered_uri;
    // char response_buffer[MAX_HTTP_OUTPUT_BUFFER];
    // esp_http_client_config_t config;
    // esp_http_client_handle_t client;
};
}
#endif
