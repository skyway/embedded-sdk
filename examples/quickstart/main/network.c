//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#include "network.h"

#include <string.h>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>
#include <sdkconfig.h>

static const char* TAG = "quickstart";

// Wi-Fi接続の待ち時間（ミリ秒）と再試行回数。
#define WIFI_CONNECT_TIMEOUT_MS (30U * 1000U)
#define WIFI_CONNECT_RETRY_COUNT (5U)

// SNTPの同期先。
// 本サンプルでは、pool.ntp.orgを指定しています。
// 製品/ソフトウェアとして組み込む場合には適したサーバーへ変更してください。
// https://www.ntppool.org/vendors.html
#define SNTP_SERVER "pool.ntp.org"

// SNTPの待ち時間（ミリ秒）。
#define SNTP_SYNC_TIMEOUT_MS (60U * 1000U)

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1

static EventGroupHandle_t s_wifi_events = NULL;

// Wi-Fiイベントハンドラ。
static void wifi_event_handler(void* handler_args, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    (void)handler_args;
    (void)event_data;

    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START)) {
        (void)esp_wifi_connect();
    } else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        xEventGroupSetBits(s_wifi_events, WIFI_DISCONNECTED_BIT);
    } else if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP)) {
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    } else {
        // 対象外のイベントは無視します。
    }
}

// Wi-Fiへ接続する関数。
static bool wifi_connect(void)
{
    esp_err_t err = nvs_flash_init();
    if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    if (esp_netif_create_default_wifi_sta() == NULL) {
        ESP_LOGE(TAG, "Failed to create the Wi-Fi station interface.");
        return false;
    }

    s_wifi_events = xEventGroupCreate();
    if (s_wifi_events == NULL) {
        ESP_LOGE(TAG, "Failed to create the Wi-Fi event group.");
        return false;
    }
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        wifi_event_handler, NULL, NULL));

    const wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {0};
    (void)strlcpy((char*)wifi_config.sta.ssid, CONFIG_SKYWAY_QUICKSTART_WIFI_SSID,
                  sizeof(wifi_config.sta.ssid));
    (void)strlcpy((char*)wifi_config.sta.password, CONFIG_SKYWAY_QUICKSTART_WIFI_PASSWORD,
                  sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    for (uint32_t i = 0U; i < WIFI_CONNECT_RETRY_COUNT; i++) {
        const EventBits_t bits = xEventGroupWaitBits(s_wifi_events,
                                                     WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT,
                                                     pdTRUE, pdFALSE,
                                                     pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));
        if ((bits & WIFI_CONNECTED_BIT) != 0U) {
            // 省電力（modem sleep）の場合、SkyWay Embedded SDK上での通信が不安定となるため無効にします。
            ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
            ESP_LOGI(TAG, "Wi-Fi connected. ssid=%s", CONFIG_SKYWAY_QUICKSTART_WIFI_SSID);
            return true;
        }
        ESP_LOGW(TAG, "Wi-Fi connection attempt %u failed. Retrying.", (unsigned int)(i + 1U));
        (void)esp_wifi_connect();
    }

    ESP_LOGE(TAG, "Failed to connect to Wi-Fi.");
    return false;
}

// SNTPで時刻を合わせる関数。
static bool sntp_sync(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(SNTP_SERVER);
    if (esp_netif_sntp_init(&config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SNTP.");
        return false;
    }
    const esp_err_t err = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(SNTP_SYNC_TIMEOUT_MS));
    esp_netif_sntp_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to synchronize the clock via SNTP.");
        return false;
    }

    ESP_LOGI(TAG, "Clock synchronized.");
    return true;
}

// Wi-Fiへ接続し、SNTPで時刻を合わせる関数。
bool network_start(void)
{
    if (!wifi_connect()) {
        return false;
    }
    return sntp_sync();
}
