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

#include "skyway/room/publication.h"
#include "skyway/room/subscription.h"

/// @brief LocalPerson APIのエラーコード。
typedef enum {
    SKW_ROOM_LOCAL_PERSON_OK                           = 0,   ///< 正常終了
    SKW_ROOM_LOCAL_PERSON_ERR_NULL_ARGS                = 1,   ///< 引数がNULL
    SKW_ROOM_LOCAL_PERSON_ERR_INVALID_ARGS             = 2,   ///< 引数の値が不正
    SKW_ROOM_LOCAL_PERSON_ERR_CONTEXT_NOT_INITED       = 3,   ///< Contextが初期化されていない
    SKW_ROOM_LOCAL_PERSON_ERR_CONTEXT_NOT_SETUP        = 4,   ///< Contextがセットアップされていない
    SKW_ROOM_LOCAL_PERSON_ERR_REQUEST_FAILED           = 5,   ///< リクエストの実行に失敗
    SKW_ROOM_LOCAL_PERSON_ERR_MUTEX_UNLOCK_FAILED      = 6,   ///< LocalPerson APIのアンロックに失敗
    SKW_ROOM_LOCAL_PERSON_ERR_CALLED_IN_HANDLER        = 7,   ///< イベントハンドラ内から呼ばれた
    SKW_ROOM_LOCAL_PERSON_ERR_SEND_FAILED              = 8,   ///< リクエストの送信に失敗
    SKW_ROOM_LOCAL_PERSON_ERR_TIMEOUT                  = 9,   ///< 応答の受信がタイムアウトした
    SKW_ROOM_LOCAL_PERSON_ERR_CONNECTION_CLOSED        = 10,  ///< 応答待ちの間に接続が切断された
    SKW_ROOM_LOCAL_PERSON_ERR_RESPONSE_TOO_LARGE       = 11,  ///< 応答が受信バッファのサイズを超過した
    SKW_ROOM_LOCAL_PERSON_ERR_RESPONSE_PARSE_FAILED    = 12,  ///< 応答の解析に失敗した
    SKW_ROOM_LOCAL_PERSON_ERR_PUBLICATION_LIMIT        = 13,  ///< Publishできる数の上限を超過
    SKW_ROOM_LOCAL_PERSON_ERR_NOT_JOINED               = 14,  ///< LocalPersonが存在しない
    SKW_ROOM_LOCAL_PERSON_ERR_STREAM_FAILED            = 15,  ///< Streamの確立に失敗した
    SKW_ROOM_LOCAL_PERSON_ERR_VIDEO_SUBSCRIBE_DISABLED = 16,  ///< VideoのSubscribeが無効（Contextのオプションで許可していない）
    SKW_ROOM_LOCAL_PERSON_ERR_NOT_SUPPORTED            = 90,  ///< 未サポートの機能の呼び出し
    SKW_ROOM_LOCAL_PERSON_ERR_UNKNOWN                  = 99,  ///< 不明なエラー
    SKW_ROOM_LOCAL_PERSON_ERR_BAD_REQUEST              = 400, ///< リクエストパラメータが不正
    SKW_ROOM_LOCAL_PERSON_ERR_UNAUTHORIZED             = 401, ///< AuthTokenが不正、もしくは有効期限切れ
    SKW_ROOM_LOCAL_PERSON_ERR_FORBIDDEN                = 403, ///< 必要な権限が無い
    SKW_ROOM_LOCAL_PERSON_ERR_NOT_FOUND                = 404, ///< リソースが見つからない
    SKW_ROOM_LOCAL_PERSON_ERR_ALREADY_EXIST            = 409, ///< すでに存在するリソースを作成しようとした
    SKW_ROOM_LOCAL_PERSON_ERR_TOO_LARGE                = 413, ///< リクエストパラメータが大きすぎる
    SKW_ROOM_LOCAL_PERSON_ERR_TOO_MANY_REQUESTS        = 429, ///< レートリミット超過
    SKW_ROOM_LOCAL_PERSON_ERR_SERVER_INTERNAL          = 500, ///< サーバー内部エラー
    SKW_ROOM_LOCAL_PERSON_ERR_SERVER_BUSY              = 503, ///< サーバーが過負荷で一時的に利用できない
} skw_room_local_person_err_t;

/// @brief skw_room_local_person_publishのパラメータ。
typedef struct {
    const char* room_id;                                ///< PublishするRoomID
    const char* member_id;                              ///< PublishするMember（LocalPerson）のID
    skw_room_publication_content_type_t content_type;   ///< コンテンツ種別（Video/Audio/Data）
    skw_room_publication_type_t type;                   ///< 配信方式
    const char* metadata;                               ///< PublicationMetadata
} skw_room_local_person_publish_params_t;

/// @brief skw_room_local_person_publish_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_LOCAL_PERSON_PUBLISH_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_local_person_unpublishのパラメータ。
typedef struct {
    const char* room_id;        ///< 対象のRoomID
    const char* publication_id; ///< UnpublishするPublicationのID
} skw_room_local_person_unpublish_params_t;

/// @brief skw_room_local_person_unpublish_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_LOCAL_PERSON_UNPUBLISH_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_local_person_subscribeのパラメータ。
typedef struct {
    const char* room_id;        ///< SubscribeするRoomID
    const char* member_id;      ///< SubscribeするMember（LocalPerson）のID
    const char* publication_id; ///< Subscribe対象のPublicationのID
} skw_room_local_person_subscribe_params_t;

/// @brief skw_room_local_person_subscribe_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_LOCAL_PERSON_SUBSCRIBE_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_local_person_unsubscribeのパラメータ。
typedef struct {
    const char* room_id;         ///< 対象のRoomID
    const char* subscription_id; ///< UnsubscribeするSubscriptionのID
} skw_room_local_person_unsubscribe_params_t;

/// @brief skw_room_local_person_unsubscribe_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_LOCAL_PERSON_UNSUBSCRIBE_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief StreamをPublishします。
/// @param params Publishパラメータのポインタ
/// @param data_fields PublishされたPublicationの情報を格納する出力先のポインタ（不要ならNULL）。
/// @return エラーコード
skw_room_local_person_err_t skw_room_local_person_publish(const skw_room_local_person_publish_params_t* params, skw_room_publication_data_field_t* data_fields);

/// @brief PublishしているStreamをUnpublishします。
/// @param params Unpublishパラメータのポインタ
/// @return エラーコード
skw_room_local_person_err_t skw_room_local_person_unpublish(const skw_room_local_person_unpublish_params_t* params);

/// @brief PublicationをSubscribeし、Streamが利用可能になるまでブロックします（最大60秒）。
///        タイムアウト・確立失敗時はSubscriptionを自動でUnsubscribeします。
/// @param params Subscribeパラメータのポインタ
/// @param data_fields SubscribeされたSubscriptionの情報を格納する出力先のポインタ（不要ならNULL）。
/// @return エラーコード
skw_room_local_person_err_t skw_room_local_person_subscribe(const skw_room_local_person_subscribe_params_t* params, skw_room_subscription_data_field_t* data_fields);

/// @brief SubscribeしているStreamをUnsubscribeします。
/// @param params Unsubscribeパラメータのポインタ
/// @return エラーコード
skw_room_local_person_err_t skw_room_local_person_unsubscribe(const skw_room_local_person_unsubscribe_params_t* params);

#ifdef __cplusplus
}
#endif
