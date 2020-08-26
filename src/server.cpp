#include <vector>
#include <algorithm>

#include <esp_log.h>
#include <esp_system.h>
#include <esp_http_server.h>

#include <server.hpp>

#define TAG HTTP_TAG

LDM::Server::Server(char* base_path) {
// LDM::Server::Server() {
    this->context = (rest_server_context_t*)calloc(1, sizeof(rest_server_context_t));
    if(!this->context) {
        ESP_LOGE(TAG, "No memory for rest context");
        return;
    }

    // strlcpy(this->context->base_path, base_path, sizeof(this->context->base_path));

    this->server = NULL;
    this->config = HTTPD_DEFAULT_CONFIG();
    this->config.uri_match_fn = httpd_uri_match_wildcard;
}

LDM::Server::~Server(void) {
    free(this->context);
    // stopping server will remove all uris
    this->stopServer();
    this->registered_uri.clear();
}

httpd_config_t * LDM::Server::getConfig(void) {
    return &this->config;
}

esp_err_t LDM::Server::stopServer(void) {
    esp_err_t err = ESP_OK;
    if(this->server != NULL) {
        err = httpd_stop(this->server);
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "Error Stopping Server %s", esp_err_to_name(err));
            return err;
        }
    } else {
        ESP_LOGE(TAG, "Error Stopping Server, Server not started");
    }
    return err;
}

esp_err_t LDM::Server::startServer(void) {
    ESP_LOGI(TAG, "Starting HTTP Server");
    esp_err_t err = httpd_start(&this->server, &this->config);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error Starting Server %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

esp_err_t LDM::Server::registerUriHandle(httpd_uri_t *uri) {
    if(this->server == NULL) {
        ESP_LOGE(TAG, "Error registering URI: Server is not initialized");
        return ESP_FAIL;
    }

    esp_err_t err = httpd_register_uri_handler(this->server, uri);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error registering URI %s : %s", uri->uri, esp_err_to_name(err));
        return err;
    }

    this->registered_uri.push_back(uri);
    return err;
}

esp_err_t LDM::Server::unregisterUriHandle(httpd_uri_t *uri) {
    if(this->server == NULL) {
        ESP_LOGE(TAG, "Error registering URI: Server is not initialized");
        return ESP_FAIL;
    }

    esp_err_t err = httpd_unregister_uri_handler(this->server, uri->uri, uri->method);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error unregistering URI %s : %s", uri->uri, esp_err_to_name(err));
    }
    this->registered_uri.erase(
        std::remove(this->registered_uri.begin(),
            this->registered_uri.end(), uri),
        this->registered_uri.end()
    );
    return err;
}

esp_err_t LDM::Server::unregisterUriHandle(char* uri, httpd_method_t method) {
    if(this->server == NULL) {
        ESP_LOGE(TAG, "Error registering URI: Server is not initialized");
        return ESP_FAIL;
    }

    esp_err_t err = httpd_unregister_uri_handler(this->server, uri, method);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error unregistering URI %s : %s", uri, esp_err_to_name(err));
    }

    // populate list of handlers to remove
    std::vector<int> eraseList;
    for(int i=0; i < this->registered_uri.size(); i++) {
            err |= httpd_unregister_uri_handler(this->server, uri, method);
            if(this->registered_uri[i]->method == method && strcmp(this->registered_uri[i]->uri, uri)==0) {
            eraseList.push_back(i);
            if(err != ESP_OK) {
                ESP_LOGE(TAG, "Error unregistering URI %s : %s", uri, esp_err_to_name(err));
            }
        }
    }

    // update handler list
    int cnt = 0;
    for(auto i : eraseList) {
        this->registered_uri.erase(this->registered_uri.begin()+i - cnt++);
    }
    // this->registered_uri.erase(
    //     std::remove(this>registered_uri.begin(),
    //         this->registered_uri.end(), handle),
    //     this->registered_uri.end()
    // );
    return err;
}

esp_err_t LDM::Server::unregisterAllUriHandles(void) {
    if(this->server == NULL) {
        ESP_LOGE(TAG, "Error registering URI: Server is not initialized");
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;

    // remove all handlers
    for(auto uri : this->registered_uri) {
        err |= httpd_unregister_uri(this->server, uri->uri);
        if(err != ESP_OK) {
            ESP_LOGE(TAG, "Error unregistering URI %s : %s", uri->uri, esp_err_to_name(err));
        }
    }

    // update handler list
    this->registered_uri.clear();
    return err;
}

esp_err_t LDM::Server::unregisterAllUriHandles(char* uri) {
    if(this->server == NULL) {
        ESP_LOGE(TAG, "Error registering URI: Server is not initialized");
        return ESP_FAIL;
    }

    esp_err_t err = ESP_OK;

    err |= httpd_unregister_uri(this->server, uri);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error unregistering URI %s : %s", uri, esp_err_to_name(err));
    }

    // populate list of handlers to remove
    std::vector<int> eraseList;
    for(int i=0; i < this->registered_uri.size(); i++) {
        if(strcmp(this->registered_uri[i]->uri, uri) == 0) {
            eraseList.push_back(i);
        }
    }

    // update handler list
    int cnt = 0;
    for(auto i : eraseList) {
        this->registered_uri.erase(this->registered_uri.begin()+i - cnt++);
    }
    return err;
}
