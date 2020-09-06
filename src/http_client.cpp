#include <cstdlib>
#include <esp_log.h>
#include <esp_event.h>

// JSON formatting
#include <cJSON.h>

// HTTP request
#include <esp_tls.h>
#include <esp_http_client.h>

// project headers
#include <http_client.hpp>

#define HTTP_TAG "LDM-LIB:HTTP_CLIENT"

LDM::HTTP_Client::HTTP_Client(char* URL) {
    // create http client
    this->config = {
        .url = URL,
        .user_data = this->response_buffer,
    };
    this->client = esp_http_client_init(&this->config);
}

esp_err_t LDM::HTTP_Client::postJSON(cJSON *message, size_t size) {
    ESP_LOGI(HTTP_TAG, "Running post_json");
    esp_err_t err = ESP_OK;

    if(message != NULL) {

        char *post_data = NULL;
        if(size == 0) {
            post_data = cJSON_Print(message);
        } else {
            post_data = cJSON_PrintBuffered(message, size, 1);
            // post_data = (char*)malloc(sizeof(char)*size);
            // cJSON_PrintPreallocated(message, post_data, size, 1);
        }

        // post data
        this->postFormattedJSON(post_data);

        // free memory
        cJSON_free((void*)post_data);
        post_data = NULL;
    } else {
        ESP_LOGE(HTTP_TAG, "JSON Message is NULL");
    }
    return err;
}

esp_err_t LDM::HTTP_Client::postFormattedJSON(char *message) {
    esp_err_t err = ESP_OK;

    ESP_LOGI(HTTP_TAG, "Sending JSON Message: %s", message);

    if(message != NULL) {
        esp_http_client_set_method(this->client, HTTP_METHOD_POST);
        esp_http_client_set_header(this->client, "Content-Type", "application/json");
        esp_http_client_set_post_field(this->client, message, strlen(message));

        // post JSON message
        err = esp_http_client_perform(this->client);
        if (err == ESP_OK) {
            ESP_LOGI(HTTP_TAG, "HTTP_Client POST Status = %d, content_length = %d",
                    esp_http_client_get_status_code(this->client),
                    esp_http_client_get_content_length(this->client));
        } else {
            ESP_LOGE(HTTP_TAG, "HTTP_Client POST request failed: %s", esp_err_to_name(err));
        }
    } else {
        ESP_LOGE(HTTP_TAG, "Formatted Message is NULL");
    }
    return err;
}
