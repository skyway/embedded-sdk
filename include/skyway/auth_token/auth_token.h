//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

/// @brief AuthTokenを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_AUTH_TOKEN_MAX_TOKEN_SIZE (4096U)

/// @brief AuthToken APIのエラーコード。
typedef enum {
    SKW_AUTH_TOKEN_OK                   = 0, ///< 正常終了
    SKW_AUTH_TOKEN_ERR_NULL_ARGS        = 1, ///< 引数がNULL
    SKW_AUTH_TOKEN_ERR_BUFFER_OVERFLOW  = 2, ///< 出力バッファが不足
    SKW_AUTH_TOKEN_ERR_ENCODE_FAILED    = 3, ///< AuthTokenの生成に失敗
    SKW_AUTH_TOKEN_ERR_INVALID_ARGS     = 4, ///< 引数の値が不正
    SKW_AUTH_TOKEN_ERR_CLOCK_INVALID    = 5, ///< システム時刻が不正
} skw_auth_token_err_t;

/// @brief 生成したAuthTokenを保持する構造体。
typedef struct {
    char str[SKW_AUTH_TOKEN_MAX_TOKEN_SIZE]; ///< NUL終端済みのAuthToken文字列
    size_t len;                              ///< str内の文字列長（NUL終端を含まない）
} skw_auth_token_t;

/// @brief SkyWayで利用するAuthTokenを生成します。このAuthTokenは開発用であり、商用環境での使用は推奨されません。
/// @param out_token 出力先のAuthTokenのポインタ
/// @param app_id アプリケーションID
/// @param secret_key シークレットキー
/// @return エラーコード
skw_auth_token_err_t skw_auth_token_generate_for_dev(
    skw_auth_token_t *out_token,
    const char *app_id,
    const char *secret_key);

#ifdef __cplusplus
}
#endif
