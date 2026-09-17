//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#include "skyway/auth_token/auth_token.h"

/// @brief skw_context_setupが出力を抑制するログのタグ。
#define SKW_CONTEXT_SUPPRESS_LOG_TAGS {"AGENT", "BUF_MNGR", "PEER_DEF", "DTLS"}

/// @brief Context APIのエラーコード。
typedef enum {
    SKW_CONTEXT_OK                           = 0,   ///< 正常終了
    SKW_CONTEXT_ERR_NULL_ARGS                = 1,   ///< 引数がNULL
    SKW_CONTEXT_ERR_NOT_INITED               = 2,   ///< Contextが初期化されていない
    SKW_CONTEXT_ERR_ALREADY_INITED           = 3,   ///< Contextがすでに初期化済み
    SKW_CONTEXT_ERR_DISCONNECT_FAILED        = 4,   ///< サーバーからの切断に失敗
    SKW_CONTEXT_ERR_NOT_SETUP                = 5,   ///< Contextがセットアップされていない
    SKW_CONTEXT_ERR_ALREADY_SETUP            = 6,   ///< Contextがすでにセットアップ済み
    SKW_CONTEXT_ERR_API_TIMEOUT              = 7,   ///< Context APIがタイムアウトした
    SKW_CONTEXT_ERR_MUTEX_UNLOCK_FAILED      = 8,   ///< Context APIのアンロックに失敗
    SKW_CONTEXT_ERR_APP_ID_TOO_LONG          = 9,   ///< AppIDが長すぎる
    SKW_CONTEXT_ERR_INVALID_TOKEN            = 10,  ///< AuthTokenが不正
    SKW_CONTEXT_ERR_CALLED_IN_HANDLER        = 11,  ///< イベントハンドラ内から呼ばれた
    SKW_CONTEXT_ERR_APP_ID_MISMATCH          = 12,  ///< AuthToken内のappIdが現在のappIdと一致しない
    SKW_CONTEXT_ERR_SEND_FAILED              = 13,  ///< リクエストの送信に失敗
    SKW_CONTEXT_ERR_TIMEOUT                  = 14,  ///< 応答の受信がタイムアウトした
    SKW_CONTEXT_ERR_CONNECTION_CLOSED        = 15,  ///< 応答待ちの間に接続が切断された
    SKW_CONTEXT_ERR_RESPONSE_TOO_LARGE       = 16,  ///< 応答が受信バッファのサイズを超過した
    SKW_CONTEXT_ERR_RESPONSE_PARSE_FAILED    = 17,  ///< 応答の解析に失敗した
    SKW_CONTEXT_ERR_SIGNALING_NOT_ACTIVE     = 18,  ///< シグナリングのセッションが確立していない
    SKW_CONTEXT_ERR_SIGNALING_DISCONNECTED   = 19,  ///< シグナリングが切断された（復帰はdispose後のsetup/join）
    SKW_CONTEXT_ERR_SIGNALING_FAILED         = 20,  ///< シグナリングの操作に失敗
    SKW_CONTEXT_ERR_UNKNOWN                  = 99,  ///< 不明なエラー
    SKW_CONTEXT_ERR_BAD_REQUEST              = 400, ///< 不正なリクエスト
    SKW_CONTEXT_ERR_UNAUTHORIZED             = 401, ///< AuthTokenが不正、もしくは有効期限切れ
    SKW_CONTEXT_ERR_FORBIDDEN                = 403, ///< 必要な権限が無い
    SKW_CONTEXT_ERR_NOT_FOUND                = 404, ///< リソースが見つからない
    SKW_CONTEXT_ERR_ALREADY_EXIST            = 409, ///< すでに存在するリソースを作成しようとした
    SKW_CONTEXT_ERR_TOO_LARGE                = 413, ///< リクエストが大きすぎる
    SKW_CONTEXT_ERR_TOO_MANY_REQUESTS        = 429, ///< レートリミット超過
    SKW_CONTEXT_ERR_SERVER_INTERNAL          = 500, ///< サーバー内部エラー
    SKW_CONTEXT_ERR_SERVER_BUSY              = 503, ///< サーバーが過負荷で一時的に利用できない
    SKW_CONTEXT_FATAL_ALLOCATE_FAILED        = -1,  ///< 致命的エラー: メモリ確保に失敗
    SKW_CONTEXT_FATAL_CLEANUP_FAILED         = -2,  ///< 致命的エラー: クリーンアップに失敗
    SKW_CONTEXT_FATAL_SETUP_FAILED           = -3,  ///< 致命的エラー: セットアップに失敗
    SKW_CONTEXT_FATAL_CONNECT_FAILED         = -4,  ///< 致命的エラー: サーバーへの接続に失敗
    SKW_CONTEXT_FATAL_INIT_FAILED            = -5,  ///< 致命的エラー: 初期化に失敗
} skw_context_err_t;

/// @brief Contextの初期化時に使用するオプション。
typedef struct {
    bool disable_log_suppression; ///< SKW_CONTEXT_SUPPRESS_LOG_TAGSのタグの出力を抑制しません。

    /// @brief 試験的機能。利用の際には注意の必要な挙動や制約が含まれることがあります。
    struct {
        bool enable_video_subscribe; ///< VideoのSubscribeを許可します。Videoの受信は通信処理の負荷が高く注意が必要です。
    } experimental;
} skw_context_setup_options_t;

/// @brief Contextを初期化します。他のAPIを利用する前に最初に一度呼び出す必要があります。
/// @return エラーコード
skw_context_err_t skw_context_init(void);

/// @brief Contextを初期化し、SkyWayサーバーへの接続を確立します。
///        disable_log_suppressionを指定しない場合、SKW_CONTEXT_SUPPRESS_LOG_TAGSのタグの出力を止めます。
/// @param auth_token 認証に使用するAuthTokenのポインタ
/// @param options 初期化時に利用するオプションのポインタ
/// @return エラーコード
skw_context_err_t skw_context_setup(skw_auth_token_t* auth_token, skw_context_setup_options_t* options);

/// @brief SkyWayサーバーとの通信を切断、確保したリソースを解放します。
/// @return エラーコード
skw_context_err_t skw_context_dispose(void);

/// @brief 認証に使用するAuthTokenを新しいAuthTokenへ更新します。
/// @param auth_token 新しいAuthTokenのポインタ
/// @return エラーコード
skw_context_err_t skw_context_update_auth_token(skw_auth_token_t* auth_token);

#ifdef __cplusplus
}
#endif
