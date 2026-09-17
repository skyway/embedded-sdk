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
