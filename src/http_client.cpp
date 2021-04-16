#include <cstdlib>
#include <string>

#include <esp_log.h>
#include <esp_event.h>
#include <esp_err.h>

// JSON formatting
#include <cJSON.h>

// HTTP request
#include <esp_tls.h>
#include <esp_http_client.h>

// project headers
#include <http_client.hpp>

#define HTTP_TAG "LDM-LIB:HTTP_CLIENT"

LDM::HTTP_Client::HTTP_Client(char* URL) {
    this->URL = std::string(URL);

    // create http client
    this->config = {
        .url = this->URL.c_str(),
        .user_data = this->response_buffer,
    };
    // this->client = esp_http_client_init(&this->config);
}

LDM::HTTP_Client::~HTTP_Client(void) {
    this->deinit();
}

esp_http_client_handle_t LDM::HTTP_Client::getClient(void) {
    return this->client;
}

const char * LDM::HTTP_Client::getURL(void) {
    return this->URL.c_str();
}

std::string LDM::HTTP_Client::getURLString(void) {
    return this->URL;
}

esp_err_t LDM::HTTP_Client::deinit(void) {
    esp_err_t err = ESP_OK;
    if(this->client != NULL) {
        err = esp_http_client_close(this->client);
        if(err != ESP_OK) {
            ESP_LOGE(HTTP_TAG, "Failed to close HTTP Client: %s", esp_err_to_name(err));
        }

        err = esp_http_client_cleanup(this->client);
        if(err != ESP_OK) {
            ESP_LOGE(HTTP_TAG, "Failed to cleanup HTTP Client: %s", esp_err_to_name(err));
        }

        this->client = NULL;
    }

    return err;
}

esp_err_t LDM::HTTP_Client::setURL(char* URL) {
    this->URL = std::string(URL);
    this->config = {
        .url = this->URL.c_str(),
    };

    // reinitialize client with new configuration
    this->deinit();
    this->client = esp_http_client_init(&this->config);

    return ESP_OK;
}

esp_err_t LDM::HTTP_Client::postJSON(cJSON *message, size_t size) {
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

    // size_t url_len = 0;
    // char url_string[64];
    // err = esp_http_client_get_url(this->client, &url_string, 64*sizeof(char));
    // ESP_LOGI(HTTP_TAG, "Endpoint URL Destination: %s", this->config.url);
    ESP_LOGI(HTTP_TAG, "Endpoint URL Destination: %s", this->config.url);

    // initialize http client
    this->client = esp_http_client_init(&this->config);
    if(this->client == NULL) {
        ESP_LOGE(HTTP_TAG, "Failed to initialize http client: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }

    if(message != NULL) {
        // ESP_LOGI(HTTP_TAG, "Sending JSON Message: %s", message);
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

    // cleanup http client
    err = esp_http_client_cleanup(this->client);
    if(err != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to clean up http client: %s", esp_err_to_name(err));
    }
    this->client = NULL;

    return err;
}
