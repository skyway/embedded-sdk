idf_component_register(SRCS "main.c" "media_dummy.c" "network.c"
                       INCLUDE_DIRS ""
                       REQUIRES esp_wifi esp_netif nvs_flash lwip
                                esp_http_client esp_websocket_client mbedtls esp_timer pthread)

# SkyWay Embedded SDKを組み込む。
include("${CMAKE_CURRENT_LIST_DIR}/../skyway.cmake")
