#include <cstring>

#include <esp_log.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_blufi_api.h>
#include <esp_wifi.h>

#include <bluetooth.hpp>
#include <ble.hpp>

#define TAG "LDM-LIB:BLE-DEFAULTS"

#define ERR_CHECK(_x, _msg) \
if(_x != ESP_OK) {\
    ESP_LOGE(TAG, "%s "#_msg": %s\n", __func__, esp_err_to_name(err));\
    return err;\
}

// define statics
// BluFi profile default callback info
wifi_config_t LDM::BLE::sta_config;
wifi_config_t LDM::BLE::ap_config;
uint8_t LDM::BLE::server_if;
uint16_t LDM::BLE::conn_id;

// BluFi profile: store the station info for send back to phone
bool LDM::BLE::gl_sta_connected;
bool LDM::BLE::ble_is_connected;
uint8_t LDM::BLE::gl_sta_bssid[6];
uint8_t LDM::BLE::gl_sta_ssid[32];
int LDM::BLE::gl_sta_ssid_len;
esp_ble_adv_data_t LDM::BLE::default_blufi_adv_data;
esp_ble_adv_params_t LDM::BLE::default_blufi_adv_params;


esp_err_t LDM::BLE::setupDefaultBlufiCallback(void) {
    LDM::BLE::gl_sta_connected = false;
    LDM::BLE::connected = false;

    uint8_t default_service_uuid128[32] = {
        /* LSB <--------------------------------------------------------------------------------> MSB */
        //first uuid, 16bit, [12],[13] is the value
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
    };

    LDM::BLE::default_blufi_adv_data.set_scan_rsp = false;
    LDM::BLE::default_blufi_adv_data.include_name = true;
    LDM::BLE::default_blufi_adv_data.include_txpower = true;
    LDM::BLE::default_blufi_adv_data.min_interval = 0x0006; //slave connection min interval, Time = min_interval * 1.25 msec
    LDM::BLE::default_blufi_adv_data.max_interval = 0x0010; //slave connection max interval, Time = max_interval * 1.25 msec
    LDM::BLE::default_blufi_adv_data.appearance = 0x00;
    LDM::BLE::default_blufi_adv_data.manufacturer_len = 0;
    LDM::BLE::default_blufi_adv_data.p_manufacturer_data =  NULL;
    LDM::BLE::default_blufi_adv_data.service_data_len = 0;
    LDM::BLE::default_blufi_adv_data.p_service_data = NULL;
    LDM::BLE::default_blufi_adv_data.service_uuid_len = 16;
    LDM::BLE::default_blufi_adv_data.p_service_uuid = default_service_uuid128;
    LDM::BLE::default_blufi_adv_data.flag = 0x6;

    LDM::BLE::default_blufi_adv_params.adv_int_min        = 0x100;
    LDM::BLE::default_blufi_adv_params.adv_int_max        = 0x100;
    LDM::BLE::default_blufi_adv_params.adv_type           = ADV_TYPE_IND;
    LDM::BLE::default_blufi_adv_params.own_addr_type      = BLE_ADDR_TYPE_PUBLIC;
    // LDM::BLE::default_blufi_adv_params.peer_addr            =
    // LDM::BLE::default_blufi_adv_params.peer_addr_type       =
    LDM::BLE::default_blufi_adv_params.channel_map        = ADV_CHNL_ALL;
    LDM::BLE::default_blufi_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

    esp_blufi_callbacks_t blufi_callbacks;
    blufi_callbacks.event_cb = LDM::BLE::defaultBlufiCallback;
    // esp_blufi_callbacks_t blufi_callbacks = {
    //     .event_cb = this->defaultBlufiCallback,
    //     // .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    //     // .encrypt_func = blufi_aes_encrypt,
    //     // .decrypt_func = blufi_aes_decrypt,
    //     // .checksum_func = blufi_crc_checksum,
    // }

    ESP_LOGI(TAG, "Initializing Default GAP Callback");
    esp_err_t err = this->registerGapCallback(LDM::BLE::defaultGapHandler);
    err |= this->registerBlufiCallback(&blufi_callbacks);
    ERR_CHECK(err, "Error BLE BluFi default callback failed");

    return err;
}

void LDM::BLE::defaultGapHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&LDM::BLE::default_blufi_adv_params);
        ESP_LOGI(TAG, "Started GAP Advertising");
        break;
    default:
        break;
    }
}

void LDM::BLE::defaultBlufiCallback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param) {
    /* actually, should post to blufi_task handle the procedure,
     * now, as a example, we do it more simply */
    switch (event) {
    case ESP_BLUFI_EVENT_INIT_FINISH:
        ESP_LOGI(TAG, "BluFi init finish");
        esp_ble_gap_config_adv_data(&LDM::BLE::default_blufi_adv_data);
        break;
    case ESP_BLUFI_EVENT_DEINIT_FINISH:
        ESP_LOGI(TAG, "BluFi deinit finish");
        break;
    case ESP_BLUFI_EVENT_BLE_CONNECT:
        ESP_LOGI(TAG, "BluFi ble connect");
        // LDM::BLE::connected = true;
        LDM::BLE::server_if = param->connect.server_if;
        LDM::BLE::conn_id = param->connect.conn_id;
        esp_ble_gap_stop_advertising();
        // blufi_security_init();
        break;
    case ESP_BLUFI_EVENT_BLE_DISCONNECT:
        ESP_LOGI(TAG, "BluFi ble disconnect");
        // LDM::BLE::connected = false;
        // blufi_security_deinit();
        esp_ble_gap_start_advertising(&LDM::BLE::default_blufi_adv_params);
        break;
    case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
        ESP_LOGI(TAG, "BluFi Set WIFI opmode %d", param->wifi_mode.op_mode);
        ESP_ERROR_CHECK( esp_wifi_set_mode(param->wifi_mode.op_mode) );
        // ESP_ERROR_CHECK( LDM::BLE::wifi.setWiFiMode(param->wifi_mode.op_mode) );
        break;
    case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
        ESP_LOGI(TAG, "BluFi request wifi connect to AP");
        /* there is no wifi callback when the device has already connected to this wifi
        so disconnect wifi before connection.
        */
        esp_wifi_disconnect();
        esp_wifi_connect();
        // LDM::BLE::wifi.disconnect();
        // LDM::BLE::wifi.connect();
        break;
    case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
        ESP_LOGI(TAG, "BluFi requset wifi disconnect from AP");
        esp_wifi_disconnect();
        // LDM::BLE::wifi.disconnect();
        break;
    case ESP_BLUFI_EVENT_REPORT_ERROR:
        ESP_LOGI(TAG, "BluFi report error, error code %d", param->report_error.state);
        esp_blufi_send_error_info(param->report_error.state);
        break;
    case ESP_BLUFI_EVENT_GET_WIFI_STATUS: {
        wifi_mode_t mode;
        esp_blufi_extra_info_t info;

        esp_wifi_get_mode(&mode);

        if(gl_sta_connected) {
            memset(&info, 0, sizeof(esp_blufi_extra_info_t));
            memcpy(info.sta_bssid, gl_sta_bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = gl_sta_ssid;
            info.sta_ssid_len = gl_sta_ssid_len;
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
        } else {
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
        }
        ESP_LOGI(TAG, "BluFi get wifi status from AP");

        break;
    }
    case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
        ESP_LOGI(TAG, "blufi close a gatt connection");
        esp_blufi_close(LDM::BLE::server_if, LDM::BLE::conn_id);
        break;
    case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
        /* TODO */
        break;
	case ESP_BLUFI_EVENT_RECV_STA_BSSID:
        memcpy(LDM::BLE::sta_config.sta.bssid, param->sta_bssid.bssid, 6);
        LDM::BLE::sta_config.sta.bssid_set = 1;
        esp_wifi_set_config(WIFI_IF_STA, &LDM::BLE::sta_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_STA, &LDM::BLE::sta_config);
        ESP_LOGI(TAG, "Recv STA BSSID %s", LDM::BLE::sta_config.sta.ssid);
        break;
	case ESP_BLUFI_EVENT_RECV_STA_SSID:
        strncpy((char *)LDM::BLE::sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        LDM::BLE::sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &LDM::BLE::sta_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_STA, &LDM::BLE::sta_config);
        ESP_LOGI(TAG, "Recv STA SSID %s", LDM::BLE::sta_config.sta.ssid);
        break;
	case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
        strncpy((char *)LDM::BLE::sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
        LDM::BLE::sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &LDM::BLE::sta_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_STA, &LDM::BLE::sta_config);
        ESP_LOGI(TAG, "Recv STA PASSWORD %s", LDM::BLE::sta_config.sta.password);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
        strncpy((char *)LDM::BLE::ap_config.ap.ssid, (char *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
        LDM::BLE::ap_config.ap.ssid[param->softap_ssid.ssid_len] = '\0';
        LDM::BLE::ap_config.ap.ssid_len = param->softap_ssid.ssid_len;
        esp_wifi_set_config(WIFI_IF_AP, &LDM::BLE::ap_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_AP, &LDM::BLE::ap_config);
        ESP_LOGI(TAG, "Recv SOFTAP SSID %s, ssid len %d", LDM::BLE::ap_config.ap.ssid, LDM::BLE::ap_config.ap.ssid_len);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
        strncpy((char *)LDM::BLE::ap_config.ap.password, (char *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
        LDM::BLE::ap_config.ap.password[param->softap_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_AP, &LDM::BLE::ap_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_AP, &LDM::BLE::ap_config);
        ESP_LOGI(TAG, "Recv SOFTAP PASSWORD %s len = %d", LDM::BLE::ap_config.ap.password, param->softap_passwd.passwd_len);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
        if (param->softap_max_conn_num.max_conn_num > 4) {
            return;
        }
        LDM::BLE::ap_config.ap.max_connection = param->softap_max_conn_num.max_conn_num;
        esp_wifi_set_config(WIFI_IF_AP, &LDM::BLE::ap_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_AP, &LDM::BLE::ap_config);
        ESP_LOGI(TAG, "Recv SOFTAP MAX CONN NUM %d", LDM::BLE::ap_config.ap.max_connection);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
        if (param->softap_auth_mode.auth_mode >= WIFI_AUTH_MAX) {
            return;
        }
        LDM::BLE::ap_config.ap.authmode = param->softap_auth_mode.auth_mode;
        esp_wifi_set_config(WIFI_IF_AP, &LDM::BLE::ap_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_AP, &LDM::BLE::ap_config);
        ESP_LOGI(TAG, "Recv SOFTAP AUTH MODE %d", LDM::BLE::ap_config.ap.authmode);
        break;
	case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
        if (param->softap_channel.channel > 13) {
            return;
        }
        LDM::BLE::ap_config.ap.channel = param->softap_channel.channel;
        esp_wifi_set_config(WIFI_IF_AP, &LDM::BLE::ap_config);
        // LDM::BLE::wifi.setConfig(WIFI_IF_AP, &LDM::BLE::ap_config);
        ESP_LOGI(TAG, "Recv SOFTAP CHANNEL %d", LDM::BLE::ap_config.ap.channel);
        break;
    case ESP_BLUFI_EVENT_GET_WIFI_LIST:{
        wifi_scan_config_t scanConf = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = false
        };
        ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
        break;
    }
    case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
        ESP_LOGI(TAG, "Recv Custom Data %d", param->custom_data.data_len);
        esp_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);
        break;
	case ESP_BLUFI_EVENT_RECV_USERNAME:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_CA_CERT:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
        /* Not handle currently */
        break;
	case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
        /* Not handle currently */
        break;;
	case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
        /* Not handle currently */
        break;
    default:
        break;
    }
}
