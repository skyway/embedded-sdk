//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <sdkconfig.h>

#include <skyway/auth_token/auth_token.h>
#include <skyway/context/context.h>
#include <skyway/logger/logger.h>
#include <skyway/room/local_person.h>
#include <skyway/room/member.h>
#include <skyway/room/publication.h>
#include <skyway/room/room.h>
#include <skyway/room/subscription.h>

#include "media_dummy.h"
#include "network.h"

static const char* TAG = "quickstart";

// アプリ本体のタスクのスタックサイズ（バイト）と優先度。Subscribeを行うタスクも同じ値を使います。
#define APP_TASK_STACK_SIZE (16384U)
#define APP_TASK_PRIORITY (1U)

// 映像・音声の送信タスクのスタックサイズ（バイト）と優先度。送信を優先するため、SkyWay Embedded SDKの既定（優先度4）より高くします。
#define SEND_TASK_STACK_SIZE (4096U)
#define SEND_TASK_PRIORITY (5U)

// メインループの周期（ミリ秒）。1周ごとにデータを1通送ります。
#define APP_LOOP_INTERVAL_MS (1000U)

// 送信するデータメッセージの書式と最大長（NUL終端を含む）。固定の文言とMember名に、連番（uint32_tの最大10桁）を足した長さ。
#define DATA_MESSAGE_PREFIX "hello from " CONFIG_SKYWAY_QUICKSTART_MEMBER_NAME " #"
#define DATA_MESSAGE_SIZE (sizeof(DATA_MESSAGE_PREFIX) + 10U)

// 参加したRoomと自分のMember。
static skw_room_id_t s_room_id = {0};
static skw_room_member_id_t s_member_id = {0};

// PublishしたStreamのPublicationID。
static skw_room_publication_id_t s_video_publication_id = {0};
static skw_room_publication_id_t s_audio_publication_id = {0};
static skw_room_publication_id_t s_data_publication_id = {0};

// Subscribeする対象のPublicationIDを、イベントハンドラからSubscribeを行うタスクへ渡すキュー。
static QueueHandle_t s_subscribe_queue = NULL;

// 映像・音声の送信タスクに渡す、ダミーStreamの送り方。
typedef struct {
    const char* task_name;
    const uint8_t* data;                                                                    // フレームデータの先頭
    const media_dummy_frame_info_t* frames;                                                 // 各フレームの位置
    const uint32_t* frame_count;
    uint32_t interval_ms;                                                                   // フレーム間隔
    const skw_room_publication_id_t* publication_id;
    skw_room_publication_err_t (*send)(const skw_room_publication_send_frame_params_t* params);
} media_sender_t;
static const media_sender_t s_video_sender = {
    "video", g_media_dummy_video_data, g_media_dummy_video_frames, &g_media_dummy_video_frame_count,
    MEDIA_DUMMY_VIDEO_FRAME_INTERVAL_MS, &s_video_publication_id, skw_room_publication_send_video_frame,
};
static const media_sender_t s_audio_sender = {
    "audio", g_media_dummy_audio_data, g_media_dummy_audio_frames, &g_media_dummy_audio_frame_count,
    MEDIA_DUMMY_AUDIO_FRAME_INTERVAL_MS, &s_audio_publication_id, skw_room_publication_send_audio_frame,
};

// --------------------------
//      SkyWayの操作
// --------------------------

// Contextを初期化し、SkyWayサーバーへ接続する関数。
static bool skyway_start(void)
{
    if (skw_context_init() != SKW_CONTEXT_OK) {
        ESP_LOGE(TAG, "Failed to initialize the context.");
        return false;
    }
    skw_auth_token_t auth_token = {0};
    if (skw_auth_token_generate_for_dev(&auth_token, CONFIG_SKYWAY_QUICKSTART_APP_ID,
                                        CONFIG_SKYWAY_QUICKSTART_SECRET_KEY) != SKW_AUTH_TOKEN_OK) {
        ESP_LOGE(TAG, "Failed to generate the auth token. Check the AppID and the SecretKey.");
        return false;
    }
    if (skw_context_setup(&auth_token, NULL) != SKW_CONTEXT_OK) {
        ESP_LOGE(TAG, "Failed to setup the context.");
        return false;
    }
    return true;
}

// Roomを探すか作成し、参加する関数。前回の実行が残った同名のMemberが居れば、先に退出させます。
static bool room_join(void)
{
    skw_room_member_id_t member_ids[SKW_ROOM_CAPACITY_MEMBERS] = {0};
    skw_room_member_name_t member_names[SKW_ROOM_CAPACITY_MEMBERS] = {0};
    skw_room_member_data_fields_t members = {0};
    for (uint32_t i = 0U; i < SKW_ROOM_CAPACITY_MEMBERS; i++) {
        members[i].id = &member_ids[i];
        members[i].name = &member_names[i];
    }
    uint32_t member_count = 0U;
    skw_room_name_t room_name = {0};
    skw_room_data_field_t room_fields = {
        .id = &s_room_id,
        .name = &room_name,
        .members = &members,
        .member_count = &member_count,
    };

    // menuconfigで指定したルームを検索する。ルームが存在しなければ作成する。
    const skw_room_find_or_create_params_t room_params = SKW_ROOM_FIND_OR_CREATE_PARAMS_INIT(
        .name = CONFIG_SKYWAY_QUICKSTART_ROOM_NAME
    );
    const skw_room_err_t room_err = skw_room_find_or_create(&room_params, &room_fields);
    if (room_err != SKW_ROOM_OK) {
        ESP_LOGE(TAG, "Failed to find or create the room: %d", (int)room_err);
        return false;
    }

    // Join予定の名前を持つメンバーが居れば退出させます。
    for (uint32_t i = 0U; i < member_count; i++) {
        if (strcmp(member_names[i], CONFIG_SKYWAY_QUICKSTART_MEMBER_NAME) != 0) {
            continue;
        }
        const skw_room_member_leave_params_t leave_params = SKW_ROOM_MEMBER_LEAVE_PARAMS_INIT(
            .room_id = s_room_id,
            .member_id = member_ids[i]
        );
        if (skw_room_member_leave(&leave_params) == SKW_ROOM_MEMBER_OK) {
            ESP_LOGI(TAG, "Left an existing member. name=%s member_id=%s", member_names[i], member_ids[i]);
        }
    }

    skw_room_member_data_field_t member_fields = {
        .id = &s_member_id,
    };

    // menuconfigで指定した名前でルームに参加します。
    const skw_room_join_params_t join_params = SKW_ROOM_JOIN_PARAMS_INIT(
        .id = s_room_id,
        .name = CONFIG_SKYWAY_QUICKSTART_MEMBER_NAME
    );
    if (skw_room_join(&join_params, &member_fields) != SKW_ROOM_OK) {
        ESP_LOGE(TAG, "Failed to join the room.");
        return false;
    }
    ESP_LOGI(TAG, "Joined. room_name=%s room_id=%s member_id=%s", room_name, s_room_id, s_member_id);
    return true;
}

// 指定した種別のStreamをPublishする関数。
static bool publish_stream(skw_room_publication_content_type_t content_type,
                           skw_room_publication_id_t* out_publication_id)
{
    skw_room_publication_data_field_t fields = {
        .id = out_publication_id,
    };

    // 指定したcontent_typeのStreamをPublishします。
    const skw_room_local_person_publish_params_t params = SKW_ROOM_LOCAL_PERSON_PUBLISH_PARAMS_INIT(
        .room_id = s_room_id,
        .member_id = s_member_id,
        .content_type = content_type,
        .type = SKW_ROOM_PUBLICATION_TYPE_P2P
    );
    if (skw_room_local_person_publish(&params, &fields) != SKW_ROOM_LOCAL_PERSON_OK) {
        ESP_LOGE(TAG, "Failed to publish. content_type=%d", (int)content_type);
        return false;
    }

    ESP_LOGI(TAG, "Published. content_type=%d publication_id=%s", (int)content_type, *out_publication_id);
    return true;
}

// ダミーの映像・音声Streamを繰り返し送信するタスク。引数のmedia_sender_tで送り方を切り替えます。
static void media_send_task(void* pvParameters)
{
    const media_sender_t* sender = (const media_sender_t*)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t index = 0U;
    uint32_t pts_ms = 0U;

    while (true) {
        const media_dummy_frame_info_t* frame = &sender->frames[index];
        const skw_room_publication_send_frame_params_t params = SKW_ROOM_PUBLICATION_SEND_FRAME_PARAMS_INIT(
            .publication_id = *sender->publication_id,
            .data = &sender->data[frame->offset],
            .length = frame->size,
            .pts = pts_ms
        );
        const skw_room_publication_err_t err = sender->send(&params);
        if (err != SKW_ROOM_PUBLICATION_OK) {
            ESP_LOGW(TAG, "Failed to send a %s frame: %d", sender->task_name, (int)err);
        }
        index = (index + 1U) % *sender->frame_count;
        pts_ms += sender->interval_ms;
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(sender->interval_ms));
    }
}

// 指定したDataStreamのPublicationへメッセージを送る関数。
static void send_data_message(const char* publication_id, uint32_t message_count)
{
    char message[DATA_MESSAGE_SIZE];
    const int32_t length = snprintf(message, sizeof(message), DATA_MESSAGE_PREFIX "%u", (unsigned int)message_count);
    if ((length <= 0) || ((size_t)length >= sizeof(message))) {
        ESP_LOGE(TAG, "The data message does not fit in the buffer. ");
        return;
    }

    // 指定したDataStreamへメッセージを送信します。
    const skw_room_publication_send_data_params_t params = SKW_ROOM_PUBLICATION_SEND_DATA_PARAMS_INIT(
        .publication_id = publication_id,
        .data = (const uint8_t*)message,
        .length = (size_t)length
    );
    const skw_room_publication_err_t err = skw_room_publication_send_data(&params);
    if (err != SKW_ROOM_PUBLICATION_OK) {
        ESP_LOGW(TAG, "Failed to send the data: %d", (int)err);
    }
}

// --------------------------
//      Subscribe
// --------------------------

// キューから取り出したPublicationを順にSubscribeするタスクです。
static void subscribe_task(void* pvParameters)
{
    (void)pvParameters;

    while (true) {
        skw_room_publication_id_t publication_id = {0};
        if (xQueueReceive(s_subscribe_queue, publication_id, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        skw_room_subscription_id_t subscription_id = {0};
        skw_room_subscription_data_field_t fields = {
            .id = &subscription_id,
        };
        const skw_room_local_person_subscribe_params_t params = SKW_ROOM_LOCAL_PERSON_SUBSCRIBE_PARAMS_INIT(
            .room_id = s_room_id,
            .member_id = s_member_id,
            .publication_id = publication_id
        );
        // 失敗時（受信の上限超過、Videoが無効、確立できず等）はSkyWay Embedded SDKがSubscriptionを解除します。
        if (skw_room_local_person_subscribe(&params, &fields) == SKW_ROOM_LOCAL_PERSON_OK) {
            ESP_LOGI(TAG, "Subscribed. subscription_id=%s publication_id=%s", subscription_id, publication_id);
        } else {
            ESP_LOGW(TAG, "Failed to subscribe. publication_id=%s", publication_id);
        }
    }
}

// SubscribeするPublicationをキューへ積む関数。
static void enqueue_subscribe(const char* publication_id)
{
    skw_room_publication_id_t queued_id = {0};
    (void)strlcpy(queued_id, publication_id, sizeof(queued_id));
    if (xQueueSend(s_subscribe_queue, queued_id, 0) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue the subscribe. publication_id=%s", publication_id);
    }
}

// Subscribeのキューと、それを処理するタスクを作る関数。
static bool start_subscribe_task(void)
{
    // SkyWay Embedded SDKが持てるPublicationの数だけキューを確保します。
    s_subscribe_queue = xQueueCreate(SKW_ROOM_CAPACITY_PUBLICATIONS, sizeof(skw_room_publication_id_t));
    if (s_subscribe_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create the subscribe queue.");
        return false;
    }

    // Subscribeタスクを作成します。
    if (xTaskCreate(&subscribe_task, "subscribe", APP_TASK_STACK_SIZE, NULL, APP_TASK_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create the subscribe task.");
        return false;
    }
    return true;
}

// 参加した時点で存在していた自分以外のData PublicationをSubscribeする関数。以後はPublishのイベントで購読します。
static void subscribe_existing_publications(void)
{
    skw_room_publication_id_t publication_ids[SKW_ROOM_CAPACITY_PUBLICATIONS] = {0};
    skw_room_member_id_t publisher_ids[SKW_ROOM_CAPACITY_PUBLICATIONS] = {0};
    skw_room_publication_content_type_t content_types[SKW_ROOM_CAPACITY_PUBLICATIONS] = {0};
    skw_room_publication_data_fields_t publications = {0};
    for (uint32_t i = 0U; i < SKW_ROOM_CAPACITY_PUBLICATIONS; i++) {
        publications[i].id = &publication_ids[i];
        publications[i].publisher_id = &publisher_ids[i];
        publications[i].content_type = &content_types[i];
    }
    uint32_t publication_count = 0U;
    skw_room_data_field_t room_fields = {
        .publications = &publications,
        .publication_count = &publication_count,
    };

    // 参加しているRoomのPublicationを取得します。
    const skw_room_find_by_id_params_t room_params = SKW_ROOM_FIND_BY_ID_PARAMS_INIT(
        .id = s_room_id
    );
    const skw_room_err_t list_err = skw_room_find_by_id(&room_params, &room_fields);
    if (list_err != SKW_ROOM_OK) {
        ESP_LOGE(TAG, "Failed to list the publications: %d", (int)list_err);
        return;
    }

    // 取得したPublicationの中から、自分以外のData PublicationをSubscribeします。
    for (uint32_t i = 0U; i < publication_count; i++) {
        if ((strcmp(publisher_ids[i], s_member_id) != 0) && (content_types[i] == SKW_ROOM_PUBLICATION_CONTENT_TYPE_DATA)) {
            enqueue_subscribe(publication_ids[i]);
        }
    }
}

// --------------------------
//      イベントハンドラ
// --------------------------

// Memberが参加したときに呼ばれるイベント。
static void on_member_joined(const char* room_id, const char* member_id)
{
    (void)room_id;
    ESP_LOGI(TAG, "Member joined. member_id=%s", member_id);
}

// Memberが退出したときに呼ばれるイベント。
static void on_member_left(const char* room_id, const char* member_id)
{
    (void)room_id;
    ESP_LOGI(TAG, "Member left. member_id=%s", member_id);
}

// StreamがPublishされたときに呼ばれるイベント。このサンプルはDataだけをSubscribeします。
static void on_stream_published(const char* room_id, const char* publication_id, const char* publisher_id,
                                skw_room_publication_content_type_t content_type)
{
    (void)room_id;
    ESP_LOGI(TAG, "Stream published. publication_id=%s publisher_id=%s content_type=%d", publication_id, publisher_id, (int)content_type);
    if ((strcmp(publisher_id, s_member_id) != 0) && (content_type == SKW_ROOM_PUBLICATION_CONTENT_TYPE_DATA)) {
        enqueue_subscribe(publication_id);
    }
}

// DataStreamでデータを受信したときに呼ばれるイベント。
static void on_data_received(const char* subscription_id, const char* publication_id,
                             const uint8_t* data, size_t length)
{
    (void)subscription_id;
    ESP_LOGI(TAG, "Received data. publication_id=%s data=%.*s length=%u bytes",
             publication_id, (int)length, (const char*)data, (unsigned int)length);
}

// 上のイベントハンドラをSkyWay Embedded SDKへ登録する関数。
static bool register_handlers(void)
{
    const skw_room_handlers_t room_handlers = {
        .on_member_joined = on_member_joined,
        .on_member_left = on_member_left,
        .on_stream_published = on_stream_published,
    };
    if (skw_room_register_handlers(&room_handlers) != SKW_ROOM_OK) {
        ESP_LOGE(TAG, "Failed to register the room handlers.");
        return false;
    }
    const skw_room_subscription_handlers_t subscription_handlers = {
        .on_data = on_data_received,
    };
    if (skw_room_subscription_register_handlers(&subscription_handlers) != SKW_ROOM_SUBSCRIPTION_OK) {
        ESP_LOGE(TAG, "Failed to register the subscription handlers.");
        return false;
    }
    return true;
}

// --------------------------
//      エントリポイント
// --------------------------

// 準備を済ませ、メインループでデータ送信を繰り返すタスク。
static void app_task(void* pvParameters)
{
    (void)pvParameters;

    // ネットワークへの接続を開始します。
    if (!network_start()) {
        ESP_LOGE(TAG, "Stopped. Failed to connect to the network.");
        vTaskDelete(NULL);
    }

    // SkyWay Embedded SDKを初期化します。
    if (!skyway_start()) {
        ESP_LOGE(TAG, "Stopped. Failed to start SkyWay.");
        vTaskDelete(NULL);
    }

    // Subscribeを行うタスクを開始します。
    if (!start_subscribe_task()) {
        ESP_LOGE(TAG, "Stopped. Failed to start the subscribe task.");
        vTaskDelete(NULL);
    }

    // イベントハンドラをSkyWay Embedded SDKへ登録します。
    if (!register_handlers()) {
        ESP_LOGE(TAG, "Stopped. Failed to register the handlers.");
        vTaskDelete(NULL);
    }

    // ルームに参加します。
    if (!room_join()) {
        ESP_LOGE(TAG, "Stopped. Failed to join the room.");
        vTaskDelete(NULL);
    }

    // 映像・音声・データをPublishします。
    if (!publish_stream(SKW_ROOM_PUBLICATION_CONTENT_TYPE_VIDEO, &s_video_publication_id)) {
        ESP_LOGE(TAG, "Stopped. Failed to publish the video.");
        vTaskDelete(NULL);
    }
    if (!publish_stream(SKW_ROOM_PUBLICATION_CONTENT_TYPE_AUDIO, &s_audio_publication_id)) {
        ESP_LOGE(TAG, "Stopped. Failed to publish the audio.");
        vTaskDelete(NULL);
    }
    if (!publish_stream(SKW_ROOM_PUBLICATION_CONTENT_TYPE_DATA, &s_data_publication_id)) {
        ESP_LOGE(TAG, "Stopped. Failed to publish the data.");
        vTaskDelete(NULL);
    }

    // 映像と音声は専用のタスクで送り続けます。データはこのタスクのループで送ります。
    if ((xTaskCreate(&media_send_task, s_video_sender.task_name, SEND_TASK_STACK_SIZE, (void*)&s_video_sender, SEND_TASK_PRIORITY, NULL) != pdPASS) ||
            (xTaskCreate(&media_send_task, s_audio_sender.task_name, SEND_TASK_STACK_SIZE, (void*)&s_audio_sender, SEND_TASK_PRIORITY, NULL) != pdPASS)) {
        ESP_LOGE(TAG, "Stopped. Failed to create the send tasks.");
        vTaskDelete(NULL);
    }

    // 参加した時点で存在するPublicationもSubscribeします。
    subscribe_existing_publications();

    uint32_t loop_count = 0U;
    while (true) {
        // データメッセージを送信します。
        send_data_message(s_data_publication_id, loop_count);
        loop_count++;
        vTaskDelay(pdMS_TO_TICKS(APP_LOOP_INTERVAL_MS));
    }
}

void app_main(void)
{
    // SkyWay Embedded SDKの動作はInfoログで追えます。
    esp_log_level_set(SKW_LOGGER_TAG, ESP_LOG_INFO);

    if (xTaskCreate(&app_task, "app", APP_TASK_STACK_SIZE, NULL, APP_TASK_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create the app task.");
    }
}
