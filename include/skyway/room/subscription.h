//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

#include "skyway/room/member.h"
#include "skyway/room/publication.h"


/// @brief SubscriptionIDを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_SUBSCRIPTION_ID_SIZE (36+1)

/// @brief Subscription APIのエラーコード。
typedef enum {
    SKW_ROOM_SUBSCRIPTION_OK                          = 0,   ///< 正常終了
    SKW_ROOM_SUBSCRIPTION_ERR_NULL_ARGS               = 1,   ///< 引数がNULL
    SKW_ROOM_SUBSCRIPTION_ERR_INVALID_ARGS            = 2,   ///< 引数の値が不正
    SKW_ROOM_SUBSCRIPTION_ERR_CONTEXT_NOT_INITED      = 3,   ///< Contextが初期化されていない
    SKW_ROOM_SUBSCRIPTION_ERR_CONTEXT_NOT_SETUP       = 4,   ///< Contextがセットアップされていない
    SKW_ROOM_SUBSCRIPTION_ERR_REQUEST_FAILED          = 5,   ///< リクエストの実行に失敗
    SKW_ROOM_SUBSCRIPTION_ERR_MUTEX_UNLOCK_FAILED     = 6,   ///< Subscription APIのアンロックに失敗
    SKW_ROOM_SUBSCRIPTION_ERR_CALLED_IN_HANDLER       = 7,   ///< イベントハンドラ内から呼ばれた
    SKW_ROOM_SUBSCRIPTION_ERR_SEND_FAILED             = 8,   ///< リクエストの送信に失敗
    SKW_ROOM_SUBSCRIPTION_ERR_TIMEOUT                 = 9,   ///< 応答の受信がタイムアウトした
    SKW_ROOM_SUBSCRIPTION_ERR_CONNECTION_CLOSED       = 10,  ///< 応答待ちの間に接続が切断された
    SKW_ROOM_SUBSCRIPTION_ERR_RESPONSE_TOO_LARGE      = 11,  ///< 応答が受信バッファのサイズを超過した
    SKW_ROOM_SUBSCRIPTION_ERR_RESPONSE_PARSE_FAILED   = 12,  ///< 応答の解析に失敗した
    SKW_ROOM_SUBSCRIPTION_ERR_UNKNOWN                 = 99,  ///< 不明なエラー
    SKW_ROOM_SUBSCRIPTION_ERR_BAD_REQUEST             = 400, ///< リクエストパラメータが不正
    SKW_ROOM_SUBSCRIPTION_ERR_UNAUTHORIZED            = 401, ///< AuthTokenが不正、もしくは有効期限切れ
    SKW_ROOM_SUBSCRIPTION_ERR_FORBIDDEN               = 403, ///< 必要な権限が無い
    SKW_ROOM_SUBSCRIPTION_ERR_NOT_FOUND               = 404, ///< リソースが見つからない
    SKW_ROOM_SUBSCRIPTION_ERR_ALREADY_EXIST           = 409, ///< すでに存在するリソースを作成しようとした
    SKW_ROOM_SUBSCRIPTION_ERR_TOO_LARGE               = 413, ///< リクエストパラメータが大きすぎる
    SKW_ROOM_SUBSCRIPTION_ERR_TOO_MANY_REQUESTS       = 429, ///< レートリミット超過
    SKW_ROOM_SUBSCRIPTION_ERR_SERVER_INTERNAL         = 500, ///< サーバー内部エラー
    SKW_ROOM_SUBSCRIPTION_ERR_SERVER_BUSY             = 503, ///< サーバーが過負荷で一時的に利用できない
} skw_room_subscription_err_t;

/// @brief SubscriptionIDを格納する文字列型（NUL終端を含む）。
typedef char skw_room_subscription_id_t[SKW_ROOM_SUBSCRIPTION_ID_SIZE];

/// @brief Subscriptionの取得結果を受け取る出力先。NULLを設定したフィールドには格納されない。
typedef struct {
    skw_room_subscription_id_t *id;             ///< SubscriptionIDの格納先（不要ならNULL）
    skw_room_publication_id_t  *publication_id; ///< Subscribe対象のPublicationIDの格納先（不要ならNULL）
    skw_room_member_id_t       *subscriber_id;  ///< SubscribeしたMemberのIDの格納先（不要ならNULL）
} skw_room_subscription_data_field_t;

/// @brief SubscribeしているStreamからデータを受信したときに呼ばれます。ハンドラ内からunsubscribe等の破棄を伴うAPIとsubscribeは呼び出せません。
/// @param subscription_id 対象のSubscriptionのID
/// @param publication_id 対象のPublicationのID
/// @param data 受信したデータ（呼び出し中のみ有効）
/// @param length 受信したデータの長さ（バイト数）
typedef void (*skw_room_subscription_on_data_t)(const char* subscription_id, const char* publication_id, const uint8_t* data, size_t length);

/// @brief SubscribeしているStreamからエンコード済み映像フレーム（H.264）を受信したときに呼ばれます。ハンドラ内からunsubscribe等の破棄を伴うAPIとsubscribeは呼び出せません。
/// @param subscription_id 対象のSubscriptionのID
/// @param publication_id 対象のPublicationのID
/// @param data 受信したフレームデータ（呼び出し中のみ有効）
/// @param length 受信したフレームデータの長さ（バイト数）
/// @param pts フレームの表示タイムスタンプ（ミリ秒）
typedef void (*skw_room_subscription_on_video_frame_t)(const char* subscription_id, const char* publication_id, const uint8_t* data, size_t length, uint32_t pts);

/// @brief SubscribeしているStreamからエンコード済み音声フレーム（Opus）を受信したときに呼ばれます。ハンドラ内からunsubscribe等の破棄を伴うAPIとsubscribeは呼び出せません。
/// @param subscription_id 対象のSubscriptionのID
/// @param publication_id 対象のPublicationのID
/// @param data 受信したフレームデータ（呼び出し中のみ有効）
/// @param length 受信したフレームデータの長さ（バイト数）
/// @param pts フレームの表示タイムスタンプ（ミリ秒）
typedef void (*skw_room_subscription_on_audio_frame_t)(const char* subscription_id, const char* publication_id, const uint8_t* data, size_t length, uint32_t pts);

/// @brief Subscriptionの各イベント通知を受け取るハンドラ。不要なイベントのフィールドはNULLにできる。
typedef struct {
    skw_room_subscription_on_data_t on_data;               ///< データの受信時
    skw_room_subscription_on_video_frame_t on_video_frame; ///< 映像フレームの受信時
    skw_room_subscription_on_audio_frame_t on_audio_frame; ///< 音声フレームの受信時
} skw_room_subscription_handlers_t;

/// @brief Subscriptionのイベント通知を受け取るハンドラを登録します。登録できる集合は単一で、再登録すると上書きされます。
/// @param handlers Subscriptionのイベント通知を受け取るハンドラのポインタ
/// @return エラーコード
skw_room_subscription_err_t skw_room_subscription_register_handlers(const skw_room_subscription_handlers_t* handlers);

/// @brief 登録済みのイベント通知ハンドラを解除します。
/// @return エラーコード
skw_room_subscription_err_t skw_room_subscription_unregister_handlers(void);

#ifdef __cplusplus
}
#endif
