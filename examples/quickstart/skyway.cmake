# SkyWay Embedded SDKをアプリへ組み込むCMakeスクリプト。

set(SKYWAY_DIR "${CMAKE_CURRENT_LIST_DIR}")

# SkyWay SDKのディレクトリを設定する。
set(SKYWAY_INCLUDE_DIR "${SKYWAY_DIR}/../../include")
if(NOT EXISTS "${SKYWAY_INCLUDE_DIR}/skyway")
    message(FATAL_ERROR "SkyWay SDK: ヘッダーが見つかりません: ${SKYWAY_INCLUDE_DIR}/skyway")
endif()

# SkyWay SDKのライブラリとインクルードディレクトリを設定する。
set(SKYWAY_LIB "${SKYWAY_DIR}/libs/${IDF_TARGET}/libskyway-embedded.a")
if(NOT EXISTS "${SKYWAY_LIB}")
    file(GLOB SKYWAY_AVAILABLE_TARGETS RELATIVE "${SKYWAY_DIR}/libs" "${SKYWAY_DIR}/libs/*")
    message(FATAL_ERROR
            "SkyWay SDK: target '${IDF_TARGET}' 向けのライブラリが配布物に含まれていません。"
            " 利用できるターゲット: ${SKYWAY_AVAILABLE_TARGETS}")
endif()

# ESP-IDFのバージョンをチェックする。
set(SKYWAY_IDF_MIN_VERSION "5.5.2")
string(REGEX MATCH "^[0-9]+\\.[0-9]+" SKYWAY_IDF_SERIES "${SKYWAY_IDF_MIN_VERSION}")
set(SKYWAY_IDF_VERSION "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}")
if(NOT "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" STREQUAL "${SKYWAY_IDF_SERIES}"
   OR SKYWAY_IDF_VERSION VERSION_LESS SKYWAY_IDF_MIN_VERSION)
    message(FATAL_ERROR "SkyWay SDK: ESP-IDF v${SKYWAY_IDF_MIN_VERSION}以上の${SKYWAY_IDF_SERIES}系を使ってください"
                        "（現在 v${SKYWAY_IDF_VERSION}）。")
endif()


# ビルドに必要な設定がsdkconfigに含まれているかチェックする。
set(SKYWAY_REQUIRED_CONFIG_FILE "${SKYWAY_DIR}/skyway.sdkconfig")
if(NOT EXISTS "${SKYWAY_REQUIRED_CONFIG_FILE}")
    message(FATAL_ERROR "SkyWay SDK: skyway.sdkconfigが見つかりません。")
endif()
file(STRINGS "${SKYWAY_REQUIRED_CONFIG_FILE}" SKYWAY_REQUIRED_CONFIGS)
# 以下の設定は、skyway.sdkconfigに記載された値以上である必要がある。それ以外は完全一致である必要がある。
set(SKYWAY_MINIMUM_CONFIGS LWIP_MAX_SOCKETS MBEDTLS_SSL_IN_CONTENT_LEN PTHREAD_TASK_STACK_SIZE_DEFAULT)
set(SKYWAY_CONFIG_MISMATCHES "")
foreach(SKYWAY_REQUIRED IN LISTS SKYWAY_REQUIRED_CONFIGS)
    if("${SKYWAY_REQUIRED}" MATCHES "^CONFIG_([A-Za-z0-9_]+)=(.*)$")
        set(SKYWAY_NAME "${CMAKE_MATCH_1}")
        set(SKYWAY_EXPECTED "${CMAKE_MATCH_2}")
        set(SKYWAY_ACTUAL "${CONFIG_${SKYWAY_NAME}}")
        if("${SKYWAY_ACTUAL}" STREQUAL "")
            set(SKYWAY_ACTUAL "n")
        endif()
        if("${SKYWAY_NAME}" IN_LIST SKYWAY_MINIMUM_CONFIGS)
            if("${SKYWAY_ACTUAL}" STREQUAL "n" OR "${SKYWAY_ACTUAL}" LESS "${SKYWAY_EXPECTED}")
                list(APPEND SKYWAY_CONFIG_MISMATCHES
                     "sdkconfigの CONFIG_${SKYWAY_NAME} は ${SKYWAY_EXPECTED} 以上に設定する必要があります（現在 ${SKYWAY_ACTUAL}）")
            endif()
        elseif(NOT "${SKYWAY_ACTUAL}" STREQUAL "${SKYWAY_EXPECTED}")
            list(APPEND SKYWAY_CONFIG_MISMATCHES
                 "sdkconfigの CONFIG_${SKYWAY_NAME} は ${SKYWAY_EXPECTED} に設定する必要があります（現在 ${SKYWAY_ACTUAL}）")
        endif()
    endif()
endforeach()
if(SKYWAY_CONFIG_MISMATCHES)
    list(JOIN SKYWAY_CONFIG_MISMATCHES "。" SKYWAY_CONFIG_MISMATCHES)
    message(FATAL_ERROR "SkyWay SDK: ${SKYWAY_CONFIG_MISMATCHES}。")
endif()

# ライブラリが参照するコンポーネントの名前を解決する。
idf_build_get_property(SKYWAY_BUILD_COMPONENTS BUILD_COMPONENTS)
set(SKYWAY_LINK_REQUIRES "")
foreach(SKYWAY_DEP esp_http_client esp_netif mbedtls esp_websocket_client esp_timer pthread)
    if("espressif__${SKYWAY_DEP}" IN_LIST SKYWAY_BUILD_COMPONENTS)
        list(APPEND SKYWAY_LINK_REQUIRES "espressif__${SKYWAY_DEP}")
    else()
        list(APPEND SKYWAY_LINK_REQUIRES "${SKYWAY_DEP}")
    endif()
endforeach()

add_prebuilt_library(skyway_prebuilt "${SKYWAY_LIB}" REQUIRES ${SKYWAY_LINK_REQUIRES})
target_include_directories(${COMPONENT_LIB} PUBLIC "${SKYWAY_INCLUDE_DIR}")
target_link_libraries(${COMPONENT_LIB} PUBLIC skyway_prebuilt)
