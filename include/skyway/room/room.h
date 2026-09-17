//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "skyway/room/member.h"
#include "skyway/room/publication.h"
#include "skyway/room/subscription.h"


/// @brief RoomIDを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_ID_SIZE (36+1)
/// @brief RoomNameを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_NAME_SIZE (128+1)
/// @brief RoomMetadataを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_METADATA_SIZE (1024+1)

/// @brief 同時に接続できるRemoteMemberの数の上限。
#define SKW_ROOM_REMOTE_MEMBER_COUNT (2)

/// @brief Roomに居るMemberの数。自分と相手の合計。
#define SKW_ROOM_PERSON_COUNT (1 + SKW_ROOM_REMOTE_MEMBER_COUNT)

/// @brief Roomで想定するBotの数。SDKはSFUを使わないが、Roomの取得結果にはMemberとして現れるため枠だけ確保する。
#define SKW_ROOM_BOT_COUNT (1)

/// @brief PublishできるDataStreamの本数。Video・Audioは各1本で固定。
#define SKW_ROOM_DATA_PUBLICATION_COUNT (2)

/// @brief 相手Member1人からSubscribeできるDataStreamの本数。相手は自分より多くのDataをPublishし得るため、
///        Publishする本数とは別に決める。
#define SKW_ROOM_DATA_SUBSCRIPTION_COUNT (2)

/// @brief 1Memberが持ちうるDataStreamの本数。
#define SKW_ROOM_MEMBER_DATA_COUNT (2)

/// @brief 1MemberがPublishできるStreamの数。Video 1・Audio 1・Data。
#define SKW_ROOM_MEMBER_PUBLICATION_COUNT (1 + 1 + SKW_ROOM_MEMBER_DATA_COUNT)

/// @brief Roomの取得結果として格納できるMember数の上限。Person MemberとSFU Botの合計。
#define SKW_ROOM_CAPACITY_MEMBERS (SKW_ROOM_PERSON_COUNT + SKW_ROOM_BOT_COUNT)
/// @brief Roomの取得結果として格納できるPublication数の上限。全Person MemberがPublishできる本数の合計。
#define SKW_ROOM_CAPACITY_PUBLICATIONS (SKW_ROOM_PERSON_COUNT * SKW_ROOM_MEMBER_PUBLICATION_COUNT)
/// @brief Roomの取得結果として格納できるSubscription数の上限。各Person Memberが自分以外のPublicationを
///        すべてSubscribeした場合の合計。
#define SKW_ROOM_CAPACITY_SUBSCRIPTIONS (SKW_ROOM_PERSON_COUNT * SKW_ROOM_REMOTE_MEMBER_COUNT * SKW_ROOM_MEMBER_PUBLICATION_COUNT)

/// @brief skw_room_joinでttl_secを指定しなかった場合に使うMemberの有効期間（秒）。
#define SKW_ROOM_JOIN_TTL_SEC_DEFAULT (30U)

/// @brief Room APIのエラーコード。
typedef enum {
    SKW_ROOM_OK                          = 0,   ///< 正常終了
    SKW_ROOM_ERR_NULL_ARGS               = 1,   ///< 引数がNULL
    SKW_ROOM_ERR_INVALID_ARGS            = 2,   ///< 引数の値が不正
    SKW_ROOM_ERR_CONTEXT_NOT_INITED      = 3,   ///< Contextが初期化されていない
    SKW_ROOM_ERR_CONTEXT_NOT_SETUP       = 4,   ///< Contextがセットアップされていない
    SKW_ROOM_ERR_REQUEST_FAILED          = 5,   ///< リクエストの実行に失敗
    SKW_ROOM_ERR_MUTEX_UNLOCK_FAILED     = 6,   ///< Room APIのアンロックに失敗
    SKW_ROOM_ERR_CALLED_IN_HANDLER       = 7,   ///< イベントハンドラ内から呼ばれた
    SKW_ROOM_ERR_CAPACITY_EXHAUSTED      = 8,   ///< 取得結果が格納上限を超えた（上限までは格納済み）
    SKW_ROOM_ERR_SEND_FAILED             = 9,   ///< リクエストの送信に失敗
    SKW_ROOM_ERR_TIMEOUT                 = 10,  ///< 応答の受信がタイムアウトした
    SKW_ROOM_ERR_CONNECTION_CLOSED       = 11,  ///< 応答待ちの間に接続が切断された
    SKW_ROOM_ERR_RESPONSE_TOO_LARGE      = 12,  ///< 応答が受信バッファのサイズを超過した
    SKW_ROOM_ERR_RESPONSE_PARSE_FAILED   = 13,  ///< 応答の解析に失敗した
    SKW_ROOM_ERR_CONNECTION_FAILED       = 14,  ///< サーバーとの接続の確立に失敗
    SKW_ROOM_ERR_ALREADY_JOINED          = 15,  ///< すでにLocalPersonが存在する（disposeで解放するまで再joinできない）
    SKW_ROOM_ERR_UNKNOWN                 = 99,  ///< 不明なエラー
    SKW_ROOM_ERR_BAD_REQUEST             = 400, ///< リクエストパラメータが不正
    SKW_ROOM_ERR_UNAUTHORIZED            = 401, ///< AuthTokenが不正、もしくは有効期限切れ
    SKW_ROOM_ERR_FORBIDDEN               = 403, ///< 必要な権限が無い
    SKW_ROOM_ERR_NOT_FOUND               = 404, ///< リソースが見つからない
    SKW_ROOM_ERR_ALREADY_EXIST           = 409, ///< すでに存在するリソースを作成しようとした
    SKW_ROOM_ERR_TOO_LARGE               = 413, ///< リクエストパラメータが大きすぎる
    SKW_ROOM_ERR_TOO_MANY_REQUESTS       = 429, ///< レートリミット超過
    SKW_ROOM_ERR_SERVER_INTERNAL         = 500, ///< サーバー内部エラー
    SKW_ROOM_ERR_SERVER_BUSY             = 503, ///< サーバーが過負荷で一時的に利用できない
} skw_room_err_t;

/// @brief RoomIDを格納する文字列型（NUL終端を含む）。
typedef char skw_room_id_t[SKW_ROOM_ID_SIZE];
/// @brief RoomNameを格納する文字列型（NUL終端を含む）。
typedef char skw_room_name_t[SKW_ROOM_NAME_SIZE];
/// @brief RoomMetadataを格納する文字列型（NUL終端を含む）。
typedef char skw_room_metadata_t[SKW_ROOM_METADATA_SIZE];
/// @brief 複数のMemberの取得結果を受け取る出力先の配列型。
typedef skw_room_member_data_field_t skw_room_member_data_fields_t[SKW_ROOM_CAPACITY_MEMBERS];
/// @brief 複数のPublicationの取得結果を受け取る出力先の配列型。
typedef skw_room_publication_data_field_t skw_room_publication_data_fields_t[SKW_ROOM_CAPACITY_PUBLICATIONS];
/// @brief 複数のSubscriptionの取得結果を受け取る出力先の配列型。
typedef skw_room_subscription_data_field_t skw_room_subscription_data_fields_t[SKW_ROOM_CAPACITY_SUBSCRIPTIONS];
/// @brief 各リソースの上限超過をビットで示すフラグ。
typedef uint32_t skw_room_capacity_exhaustion_flags_t;
/// @brief Member数が上限を超過したことを示すビット。
#define SKW_ROOM_CAPACITY_EXHAUSTED_MEMBERS       (1u << 0)
/// @brief Publication数が上限を超過したことを示すビット。
#define SKW_ROOM_CAPACITY_EXHAUSTED_PUBLICATIONS  (1u << 1)
/// @brief Subscription数が上限を超過したことを示すビット。
#define SKW_ROOM_CAPACITY_EXHAUSTED_SUBSCRIPTIONS (1u << 2)

/// @brief Roomの取得/作成結果を受け取る出力先。NULLを設定したフィールドには格納されない。
typedef struct {
    skw_room_id_t                       *id;                 ///< RoomIDの格納先（不要ならNULL）
    skw_room_name_t                     *name;               ///< RoomNameの格納先（不要ならNULL）
    skw_room_metadata_t                 *metadata;           ///< RoomMetadataの格納先（不要ならNULL）
    skw_room_member_data_fields_t       *members;            ///< Member一覧の格納先（不要ならNULL）。各要素の格納先もNULL可。sideはレスポンスから判別できないため格納されない。
    uint32_t                            *member_count;       ///< 格納したMember数の格納先（不要ならNULL）
    skw_room_publication_data_fields_t  *publications;       ///< Publication一覧の格納先（不要ならNULL）。各要素の格納先もNULL可。
    uint32_t                            *publication_count;  ///< 格納したPublication数の格納先（不要ならNULL）
    skw_room_subscription_data_fields_t *subscriptions;      ///< Subscription一覧の格納先（不要ならNULL）。各要素の格納先もNULL可。
    uint32_t                            *subscription_count; ///< 格納したSubscription数の格納先（不要ならNULL）
    skw_room_capacity_exhaustion_flags_t *exhaustion_flags;  ///< 上限超過フラグの格納先（不要ならNULL）。超過したリソースを示すビットのORが格納される（超過なしは0）。
} skw_room_data_field_t;

/// @brief Roomが削除されたときに呼ばれます。
/// @param room_id 対象のRoomID
typedef void (*skw_room_on_room_closed_t)(const char* room_id);

/// @brief RoomのMetadataが更新されたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param metadata 更新後のMetadata（イベントに含まれない場合はNULL）
typedef void (*skw_room_on_room_metadata_updated_t)(const char* room_id, const char* metadata);

/// @brief Memberが参加したときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param member_id 参加したMemberのID
typedef void (*skw_room_on_member_joined_t)(const char* room_id, const char* member_id);

/// @brief Memberが退出したときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param member_id 退出したMemberのID
typedef void (*skw_room_on_member_left_t)(const char* room_id, const char* member_id);

/// @brief MemberのMetadataが更新されたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param member_id 対象のMemberのID
/// @param metadata 更新後のMetadata（イベントに含まれない場合はNULL）
typedef void (*skw_room_on_member_metadata_updated_t)(const char* room_id, const char* member_id, const char* metadata);

/// @brief StreamがPublishされたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param publication_id PublicationのID
/// @param publisher_id PublishしたMember（Publisher）のID
/// @param content_type Publicationのコンテンツ種別
typedef void (*skw_room_on_stream_published_t)(const char* room_id, const char* publication_id, const char* publisher_id,
                                               skw_room_publication_content_type_t content_type);

/// @brief StreamがUnpublishされたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param publication_id PublicationのID
/// @param publisher_id UnpublishしたMember（Publisher）のID
typedef void (*skw_room_on_stream_unpublished_t)(const char* room_id, const char* publication_id, const char* publisher_id);

/// @brief PublicationがSubscribeされたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param subscription_id SubscriptionのID
/// @param member_id SubscribeしたMemberのID
typedef void (*skw_room_on_publication_subscribed_t)(const char* room_id, const char* subscription_id, const char* member_id);

/// @brief PublicationがUnsubscribeされたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param subscription_id SubscriptionのID
/// @param member_id UnsubscribeしたMemberのID
typedef void (*skw_room_on_publication_unsubscribed_t)(const char* room_id, const char* subscription_id, const char* member_id);

/// @brief PublicationのMetadataが更新されたときに呼ばれます。
/// @param room_id 対象のRoomID
/// @param publication_id 対象のPublicationのID
/// @param metadata 更新後のMetadata（イベントに含まれない場合はNULL）
typedef void (*skw_room_on_publication_metadata_updated_t)(const char* room_id, const char* publication_id, const char* metadata);

/// @brief Roomの各イベント通知を受け取るハンドラ。不要なイベントのフィールドはNULLにできる。
typedef struct skw_room_handlers_t {
    skw_room_on_room_closed_t on_room_closed;                                   ///< Roomの削除時
    skw_room_on_room_metadata_updated_t on_room_metadata_updated;               ///< RoomのMetadata更新時
    skw_room_on_member_joined_t on_member_joined;                               ///< Memberの参加時
    skw_room_on_member_left_t on_member_left;                                   ///< Memberの退出時
    skw_room_on_member_metadata_updated_t on_member_metadata_updated;           ///< MemberのMetadata更新時
    skw_room_on_stream_published_t on_stream_published;                         ///< StreamのPublish時
    skw_room_on_stream_unpublished_t on_stream_unpublished;                     ///< StreamのUnpublish時
    skw_room_on_publication_subscribed_t on_publication_subscribed;             ///< PublicationのSubscribe時
    skw_room_on_publication_unsubscribed_t on_publication_unsubscribed;         ///< PublicationのUnsubscribe時
    skw_room_on_publication_metadata_updated_t on_publication_metadata_updated; ///< PublicationのMetadata更新時
} skw_room_handlers_t;

/// @brief skw_room_createのパラメータ。
typedef struct {
    const char* name;     ///< RoomName
    const char* metadata; ///< RoomMetadata
} skw_room_create_params_t;

/// @brief skw_room_create_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_CREATE_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_find_by_idのパラメータ。
typedef struct {
    const char* id; ///< 検索するRoomID
} skw_room_find_by_id_params_t;

/// @brief skw_room_find_by_id_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_FIND_BY_ID_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_find_by_nameのパラメータ。
typedef struct {
    const char* name; ///< 検索するRoomName
} skw_room_find_by_name_params_t;

/// @brief skw_room_find_by_name_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_FIND_BY_NAME_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_find_or_createのパラメータ。
typedef struct {
    const char* name;     ///< RoomName
    const char* metadata; ///< 新規作成時に設定するMetadata
} skw_room_find_or_create_params_t;

/// @brief skw_room_find_or_create_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_FIND_OR_CREATE_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_update_metadataのパラメータ。
typedef struct {
    const char* id;       ///< 対象のRoomID
    const char* metadata; ///< 新しいMetadata
} skw_room_update_metadata_params_t;

/// @brief skw_room_update_metadata_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_UPDATE_METADATA_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_joinのパラメータ。
typedef struct {
    const char* id;       ///< JoinするRoomID
    const char* name;     ///< MemberName（不要ならNULL）
    const char* metadata; ///< MemberMetadata（不要ならNULL）
    uint32_t ttl_sec;     ///< Memberの有効期間（秒）。0を指定するとSKW_ROOM_JOIN_TTL_SEC_DEFAULTを使う
} skw_room_join_params_t;

/// @brief skw_room_join_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_JOIN_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief 指定したパラメータでRoomを新規作成します。
/// @param params 作成パラメータのポインタ
/// @param data_fields 作成したRoomの情報を格納する出力先のポインタ
/// @return エラーコード
skw_room_err_t skw_room_create(const skw_room_create_params_t* params, skw_room_data_field_t* data_fields);

/// @brief IDでRoomを検索します。
/// @param params 検索パラメータのポインタ
/// @param data_fields 取得したRoomの情報を格納する出力先のポインタ
/// @return エラーコード
skw_room_err_t skw_room_find_by_id(const skw_room_find_by_id_params_t* params, skw_room_data_field_t* data_fields);

/// @brief NameでRoomを検索します。
/// @param params 検索パラメータのポインタ
/// @param data_fields 取得したRoomの情報を格納する出力先のポインタ
/// @return エラーコード
skw_room_err_t skw_room_find_by_name(const skw_room_find_by_name_params_t* params, skw_room_data_field_t* data_fields);

/// @brief NameでRoomを検索し、存在しない場合は新規作成します。
/// @param params 検索/作成パラメータのポインタ
/// @param data_fields 取得または作成したRoomの情報を格納する出力先のポインタ
/// @return エラーコード
skw_room_err_t skw_room_find_or_create(const skw_room_find_or_create_params_t* params, skw_room_data_field_t* data_fields);

/// @brief 指定したRoomにLocalPersonとしてJoinします。
/// @param params Joinパラメータのポインタ
/// @param data_fields JoinしたMemberの情報を格納する出力先のポインタ（不要ならNULL）。
/// @return エラーコード
skw_room_err_t skw_room_join(const skw_room_join_params_t* params, skw_room_member_data_field_t* data_fields);

/// @brief RoomのMetadataを更新します。
/// @param params 更新パラメータのポインタ
/// @return エラーコード
skw_room_err_t skw_room_update_metadata(const skw_room_update_metadata_params_t* params);

/// @brief Roomのイベント通知を受け取るハンドラを登録します。登録できる集合は単一で、再登録すると上書きされます。
/// @param handlers Roomのイベント通知を受け取るハンドラのポインタ
/// @return エラーコード
skw_room_err_t skw_room_register_handlers(const skw_room_handlers_t* handlers);

/// @brief 登録済みのイベント通知ハンドラを解除します。
/// @return エラーコード
skw_room_err_t skw_room_unregister_handlers(void);

#ifdef __cplusplus
}
#endif
