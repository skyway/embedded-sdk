//
// © NTT DOCOMO BUSINESS, Inc. All Rights Reserved.
//

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Wi-Fiへ接続し、SNTPで時刻を合わせる関数。
bool network_start(void);

#ifdef __cplusplus
}
#endif
