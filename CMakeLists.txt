# embed ca_cert certificate into binary
if(CONFIG_OTA_ENABLED)
    set(cert_pem ${project_dir}/server_certs/ca_cert.pem)
else()
    set(cert_pem "")
endif()

idf_component_register(
    SRC_DIRS src
    INCLUDE_DIRS include
    REQUIRES bt
             json
             app_update
             esp-tls
             esp_http_client
             esp_http_server
             esp_https_ota
             #esp_https_server
             driver
    EMBED_TXTFILES ${cert_pem}
)
