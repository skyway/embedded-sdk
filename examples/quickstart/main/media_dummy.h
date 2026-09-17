//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// 送信テスト用のメディア。カメラやマイクの代わりに、エンコード済みのフレームを埋め込んでいます。

// H.264Stream（カラーバー）のフレーム間隔（ミリ秒）。
#define MEDIA_DUMMY_VIDEO_FRAME_INTERVAL_MS (83U)

// OpusStream（440Hzのトーン）のフレーム間隔（ミリ秒）。
#define MEDIA_DUMMY_AUDIO_FRAME_INTERVAL_MS (60U)

// Stream内の1フレームの位置情報。映像はAccess Unit、音声は1パケットを1フレームとします。
typedef struct {
    uint32_t offset; // データ先頭からのオフセット
    uint32_t size;   // フレームのバイト数
} media_dummy_frame_info_t;

extern const uint8_t g_media_dummy_video_data[];
extern const media_dummy_frame_info_t g_media_dummy_video_frames[];
extern const uint32_t g_media_dummy_video_frame_count;

extern const uint8_t g_media_dummy_audio_data[];
extern const media_dummy_frame_info_t g_media_dummy_audio_frames[];
extern const uint32_t g_media_dummy_audio_frame_count;

#ifdef __cplusplus
}
#endif
