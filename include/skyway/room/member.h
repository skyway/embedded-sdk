//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif


/// @brief MemberIDを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_MEMBER_ID_SIZE (36+1)
/// @brief MemberNameを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_MEMBER_NAME_SIZE (128+1)
/// @brief MemberMetadataを格納するバッファのサイズ（NUL終端を含む）。
#define SKW_ROOM_MEMBER_METADATA_SIZE (1024+1)

/// @brief Member APIのエラーコード。
typedef enum {
    SKW_ROOM_MEMBER_OK                          = 0,   ///< 正常終了
    SKW_ROOM_MEMBER_ERR_NULL_ARGS               = 1,   ///< 引数がNULL
    SKW_ROOM_MEMBER_ERR_INVALID_ARGS            = 2,   ///< 引数の値が不正
    SKW_ROOM_MEMBER_ERR_CONTEXT_NOT_INITED      = 3,   ///< Contextが初期化されていない
    SKW_ROOM_MEMBER_ERR_CONTEXT_NOT_SETUP       = 4,   ///< Contextがセットアップされていない
    SKW_ROOM_MEMBER_ERR_REQUEST_FAILED          = 5,   ///< リクエストの実行に失敗
    SKW_ROOM_MEMBER_ERR_MUTEX_UNLOCK_FAILED     = 6,   ///< Member APIのアンロックに失敗
    SKW_ROOM_MEMBER_ERR_CALLED_IN_HANDLER       = 7,   ///< イベントハンドラ内から呼ばれた
    SKW_ROOM_MEMBER_ERR_SEND_FAILED             = 8,   ///< リクエストの送信に失敗
    SKW_ROOM_MEMBER_ERR_TIMEOUT                 = 9,   ///< 応答の受信がタイムアウトした
    SKW_ROOM_MEMBER_ERR_CONNECTION_CLOSED       = 10,  ///< 応答待ちの間に接続が切断された
    SKW_ROOM_MEMBER_ERR_RESPONSE_TOO_LARGE      = 11,  ///< 応答が受信バッファのサイズを超過した
    SKW_ROOM_MEMBER_ERR_RESPONSE_PARSE_FAILED   = 12,  ///< 応答の解析に失敗した
    SKW_ROOM_MEMBER_ERR_UNKNOWN                 = 99,  ///< 不明なエラー
    SKW_ROOM_MEMBER_ERR_BAD_REQUEST             = 400, ///< リクエストパラメータが不正
    SKW_ROOM_MEMBER_ERR_UNAUTHORIZED            = 401, ///< AuthTokenが不正、もしくは有効期限切れ
    SKW_ROOM_MEMBER_ERR_FORBIDDEN               = 403, ///< 必要な権限が無い
    SKW_ROOM_MEMBER_ERR_NOT_FOUND               = 404, ///< リソースが見つからない
    SKW_ROOM_MEMBER_ERR_ALREADY_EXIST           = 409, ///< すでに存在するリソースを作成しようとした
    SKW_ROOM_MEMBER_ERR_TOO_LARGE               = 413, ///< リクエストパラメータが大きすぎる
    SKW_ROOM_MEMBER_ERR_TOO_MANY_REQUESTS       = 429, ///< レートリミット超過
    SKW_ROOM_MEMBER_ERR_SERVER_INTERNAL         = 500, ///< サーバー内部エラー
    SKW_ROOM_MEMBER_ERR_SERVER_BUSY             = 503, ///< サーバーが過負荷で一時的に利用できない
} skw_room_member_err_t;

/// @brief Memberの種別。
typedef enum {
    SKW_ROOM_MEMBER_SIDE_LOCAL_PERSON  = 0, ///< LocalPerson
    SKW_ROOM_MEMBER_SIDE_REMOTE_MEMBER = 1, ///< RemoteMember
} skw_room_member_side_t;

/// @brief Memberのタイプ。
typedef enum {
    SKW_ROOM_MEMBER_TYPE_PERSON = 0, ///< Person
    SKW_ROOM_MEMBER_TYPE_BOT    = 1, ///< Bot
} skw_room_member_type_t;

/// @brief MemberIDを格納する文字列型（NUL終端を含む）。
typedef char skw_room_member_id_t[SKW_ROOM_MEMBER_ID_SIZE];
/// @brief MemberNameを格納する文字列型（NUL終端を含む）。
typedef char skw_room_member_name_t[SKW_ROOM_MEMBER_NAME_SIZE];
/// @brief MemberMetadataを格納する文字列型（NUL終端を含む）。
typedef char skw_room_member_metadata_t[SKW_ROOM_MEMBER_METADATA_SIZE];

/// @brief Memberの取得結果を受け取る出力先。NULLを設定したフィールドには格納されない。
typedef struct {
    skw_room_member_id_t       *id;       ///< MemberIDの格納先（不要ならNULL）
    skw_room_member_name_t     *name;     ///< MemberNameの格納先（不要ならNULL）
    skw_room_member_metadata_t *metadata; ///< MemberMetadataの格納先（不要ならNULL）
    skw_room_member_side_t     *side;     ///< Member種別の格納先（不要ならNULL）
    skw_room_member_type_t     *type;     ///< Memberタイプの格納先（不要ならNULL）
} skw_room_member_data_field_t;

/// @brief skw_room_member_update_metadataのパラメータ。
typedef struct {
    const char* room_id;   ///< 対象Memberが所属するRoomID
    const char* member_id; ///< 対象のMemberID
    const char* metadata;  ///< 新しいMetadata
} skw_room_member_update_metadata_params_t;

/// @brief skw_room_member_update_metadata_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_MEMBER_UPDATE_METADATA_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief skw_room_member_leaveのパラメータ。
typedef struct {
    const char* room_id;   ///< 対象Memberが所属するRoomID
    const char* member_id; ///< 退出させるMemberID
} skw_room_member_leave_params_t;

/// @brief skw_room_member_leave_params_tの初期化子。指定しなかったフィールドには既定値が入ります。
#define SKW_ROOM_MEMBER_LEAVE_PARAMS_INIT(...) {__VA_ARGS__}

/// @brief MemberのMetadataを更新します。
/// @param params 更新パラメータのポインタ
/// @return エラーコード
skw_room_member_err_t skw_room_member_update_metadata(const skw_room_member_update_metadata_params_t* params);

/// @brief MemberをRoomから退出させます。
/// @param params 退出パラメータのポインタ
/// @return エラーコード
skw_room_member_err_t skw_room_member_leave(const skw_room_member_leave_params_t* params);

#ifdef __cplusplus
}
#endif
