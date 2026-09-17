//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief SDKがログ出力時に使用するタグ。
#define SKW_LOGGER_TAG "SkyWay"

/// @brief Logger APIのエラーコード。
typedef enum {
    SKW_LOGGER_OK                       = 0, ///< 成功
    SKW_LOGGER_ERR_API_TIMEOUT          = 1, ///< Logger APIがタイムアウトした
    SKW_LOGGER_ERR_NULL_HANDLERS        = 2, ///< LoggerのハンドラがNULL
    SKW_LOGGER_ERR_NOT_INITED           = 3, ///< Loggerが初期化されていない
    SKW_LOGGER_ERR_MUTEX_UNLOCK_FAILED  = 4, ///< Loggerの排他制御のアンロックに失敗
} skw_logger_err_t;

/// @brief Loggerの通知レベル。
typedef enum {
    SKW_LOGGER_LOG_LEVEL_ERR        = 0, ///< Errorログ
    SKW_LOGGER_LOG_LEVEL_WARN       = 1, ///< Warnログ
    SKW_LOGGER_LOG_LEVEL_INFO       = 2, ///< Infoログ
    SKW_LOGGER_LOG_LEVEL_DEBUG      = 3, ///< Debugログ
    SKW_LOGGER_LOG_LEVEL_VERBOSE    = 4, ///< Verboseログ
} skw_logger_log_level_t;

/// @brief Loggerのメッセージを受け取るハンドラ。
/// @param level ログの通知レベル
/// @param tag ログのタグ
/// @param format printf形式のフォーマット文字列
/// @param args フォーマットに対応する可変引数リスト
typedef void (*skw_logger_on_message_t)(skw_logger_log_level_t level, const char* tag, const char* format, va_list args);

/// @brief Loggerのハンドラ。
typedef struct {
    skw_logger_on_message_t on_message; ///< Loggerのメッセージ受信時に呼ばれるハンドラ
} skw_logger_handlers_t;

/// @brief Loggerの出力を受け取るハンドラを登録します。登録できる集合は単一で、再登録すると上書きされます。
/// @param handlers Loggerのハンドラのポインタ
/// @return エラーコード
skw_logger_err_t skw_logger_register_handlers(const skw_logger_handlers_t* handlers);

/// @brief 登録済みのLogger出力ハンドラを解除します。
/// @return エラーコード
skw_logger_err_t skw_logger_unregister_handlers(void);

#ifdef __cplusplus
}
#endif
