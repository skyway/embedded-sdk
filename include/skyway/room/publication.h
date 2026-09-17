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


/// @brief PublicationIDを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_PUBLICATION_ID_SIZE (36+1)

/// @brief PublicationMetadataを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_PUBLICATION_METADATA_SIZE (1024+1)

/// @brief Publication APIのエラーコード。
typedef enum {
    SKW_ROOM_PUBLICATION_OK                          = 0,   ///< 正常終了
    SKW_ROOM_PUBLICATION_ERR_NULL_ARGS               = 1,   ///< 引数がNULL
    SKW_ROOM_PUBLICATION_ERR_INVALID_ARGS            = 2,   ///< 引数の値が不正
    SKW_ROOM_PUBLICATION_ERR_CONTEXT_NOT_INITED      = 3,   ///< Contextが初期化されていない
    SKW_ROOM_PUBLICATION_ERR_CONTEXT_NOT_SETUP       = 4,   ///< Contextがセットアップされていない
    SKW_ROOM_PUBLICATION_ERR_REQUEST_FAILED          = 5,   ///< リクエストの実行に失敗
    SKW_ROOM_PUBLICATION_ERR_MUTEX_UNLOCK_FAILED     = 6,   ///< Publication APIのアンロックに失敗
    SKW_ROOM_PUBLICATION_ERR_CALLED_IN_HANDLER       = 7,   ///< イベントハンドラ内から呼ばれた
    SKW_ROOM_PUBLICATION_ERR_SEND_FAILED             = 8,   ///< リクエストの送信に失敗
    SKW_ROOM_PUBLICATION_ERR_TIMEOUT                 = 9,   ///< 応答の受信がタイムアウトした
    SKW_ROOM_PUBLICATION_ERR_CONNECTION_CLOSED       = 10,  ///< 応答待ちの間に接続が切断された
    SKW_ROOM_PUBLICATION_ERR_RESPONSE_TOO_LARGE      = 11,  ///< 応答が受信バッファのサイズを超過した
    SKW_ROOM_PUBLICATION_ERR_RESPONSE_PARSE_FAILED   = 12,  ///< 応答の解析に失敗した
    SKW_ROOM_PUBLICATION_ERR_NOT_JOINED              = 13,  ///< LocalPersonが存在しない
    SKW_ROOM_PUBLICATION_ERR_CONTENT_TYPE_MISMATCH   = 14,  ///< Publicationのコンテンツ種別と送信APIが一致しない
    SKW_ROOM_PUBLICATION_ERR_NOT_PUBLISHED           = 15,  ///< 指定したPublicationをPublishしていない
    SKW_ROOM_PUBLICATION_ERR_UNKNOWN                 = 99,  ///< 不明なエラー
    SKW_ROOM_PUBLICATION_ERR_BAD_REQUEST             = 400, ///< リクエストパラメータが不正
    SKW_ROOM_PUBLICATION_ERR_UNAUTHORIZED            = 401, ///< AuthTokenが不正、もしくは有効期限切れ
    SKW_ROOM_PUBLICATION_ERR_FORBIDDEN               = 403, ///< 必要な権限が無い
    SKW_ROOM_PUBLICATION_ERR_NOT_FOUND               = 404, ///< リソースが見つからない
    SKW_ROOM_PUBLICATION_ERR_ALREADY_EXIST           = 409, ///< すでに存在するリソースを作成しようとした
    SKW_ROOM_PUBLICATION_ERR_TOO_LARGE               = 413, ///< リクエストパラメータが大きすぎる
    SKW_ROOM_PUBLICATION_ERR_TOO_MANY_REQUESTS       = 429, ///< レートリミット超過
    SKW_ROOM_PUBLICATION_ERR_SERVER_INTERNAL         = 500, ///< サーバー内部エラー
    SKW_ROOM_PUBLICATION_ERR_SERVER_BUSY             = 503, ///< サーバーが過負荷で一時的に利用できない
} skw_room_publication_err_t;

/// @brief Publicationの配信方式。
typedef enum {
    SKW_ROOM_PUBLICATION_TYPE_P2P = 0, ///< P2P
    SKW_ROOM_PUBLICATION_TYPE_SFU = 1, ///< SFU
} skw_room_publication_type_t;

/// @brief Publicationのコンテンツ種別。
typedef enum {
    SKW_ROOM_PUBLICATION_CONTENT_TYPE_VIDEO = 0, ///< 映像
    SKW_ROOM_PUBLICATION_CONTENT_TYPE_AUDIO = 1, ///< 音声
    SKW_ROOM_PUBLICATION_CONTENT_TYPE_DATA  = 2, ///< データ
} skw_room_publication_content_type_t;

/// @brief PublicationIDを格納する文字列型（NUL終端を含む）。
typedef char skw_room_publication_id_t[SKW_ROOM_PUBLICATION_ID_SIZE];
/// @brief PublicationMetadataを格納する文字列型（NUL終端を含む）。
typedef char skw_room_publication_metadata_t[SKW_ROOM_PUBLICATION_METADATA_SIZE];

/// @brief Publicationの取得結果を受け取る出力先。NULLを設定したフィールドには格納されない。
typedef struct {
    skw_room_publication_id_t           *id;           ///< PublicationIDの格納先（不要ならNULL）
    skw_room_publication_type_t         *type;         ///< 配信方式の格納先（不要ならNULL）
    skw_room_publication_content_type_t *content_type; ///< コンテンツ種別の格納先（不要ならNULL）
    skw_room_publication_metadata_t     *metadata;     ///< PublicationMetadataの格納先（不要ならNULL）
    skw_room_member_id_t                *publisher_id; ///< PublishしたMemberIDの格納先（不要ならNULL）
} skw_room_publication_data_field_t;

/// @brief skw_room_publication_update_metadataのパラメータ。
typedef struct {
    const char* room_id;        ///< 対象Publicationが属するRoomID
    const char* publication_id; ///< 対象のPublicationID
    const char* metadata;       ///< 新しいMetadata
} skw_room_publication_update_metadata_params_t;

/// @brief skw_room_publication_update_metadata_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_PUBLICATION_UPDATE_METADATA_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_publication_send_dataのパラメータ。
typedef struct {
    const char* publication_id; ///< PublishしたDataStreamのPublicationID
    const uint8_t* data;        ///< 送信するデータ
    size_t length;              ///< 送信するデータの長さ（バイト数）
} skw_room_publication_send_data_params_t;

/// @brief skw_room_publication_send_data_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_PUBLICATION_SEND_DATA_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_publication_send_video_frame/send_audio_frameのパラメータ。
typedef struct {
    const char* publication_id; ///< PublishしたVideo/AudioStreamのPublicationID
    const uint8_t* data;        ///< 送信するエンコード済みフレーム（Video: H.264 / Audio: Opus）
    size_t length;              ///< 送信するフレームの長さ（バイト数）
    uint32_t pts;               ///< フレームの表示タイムスタンプ（ミリ秒）
} skw_room_publication_send_frame_params_t;

/// @brief skw_room_publication_send_frame_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_PUBLICATION_SEND_FRAME_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief キーフレームの生成要求を受け取るハンドラ。subscriberが映像を復号できないときに呼ばれます。
/// @param publication_id 要求されたPublicationのID
typedef void (*skw_room_publication_on_keyframe_request_t)(const char* publication_id);

/// @brief Publicationの各イベント通知を受け取るハンドラ。不要なイベントのフィールドはNULLにできる。
typedef struct {
    skw_room_publication_on_keyframe_request_t on_keyframe_request; ///< キーフレームの生成要求時
} skw_room_publication_handlers_t;

/// @brief Publicationのイベント通知を受け取るハンドラを登録します。登録できる集合は単一で、再登録すると上書きされます。
/// @param handlers Publicationのイベント通知を受け取るハンドラのポインタ
/// @return エラーコード
skw_room_publication_err_t skw_room_publication_register_handlers(const skw_room_publication_handlers_t* handlers);

/// @brief 登録済みのイベント通知ハンドラを解除します。
/// @return エラーコード
skw_room_publication_err_t skw_room_publication_unregister_handlers(void);

/// @brief PublicationのMetadataを更新します。
/// @param params 更新パラメータのポインタ
/// @return エラーコード
skw_room_publication_err_t skw_room_publication_update_metadata(const skw_room_publication_update_metadata_params_t* params);

/// @brief PublishしたDataStreamへデータを送信します。Streamが確立している全subscriberへ送信されます。
///        subscriberがいない場合は何も送信せずに成功を返します。
/// @param params 送信パラメータのポインタ
/// @return エラーコード
skw_room_publication_err_t skw_room_publication_send_data(const skw_room_publication_send_data_params_t* params);

/// @brief PublishしたVideoStreamへエンコード済みフレーム（H.264）を送信します。Streamが確立している全subscriberへ送信されます。
///        subscriberがいない場合は何も送信せずに成功を返します。
/// @param params 送信パラメータのポインタ
/// @return エラーコード
skw_room_publication_err_t skw_room_publication_send_video_frame(const skw_room_publication_send_frame_params_t* params);

/// @brief PublishしたAudioStreamへエンコード済みフレーム（Opus）を送信します。Streamが確立している全subscriberへ送信されます。
///        subscriberがいない場合は何も送信せずに成功を返します。
/// @param params 送信パラメータのポインタ
/// @return エラーコード
skw_room_publication_err_t skw_room_publication_send_audio_frame(const skw_room_publication_send_frame_params_t* params);

#ifdef __cplusplus
}
#endif
