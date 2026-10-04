#pragma once
#include <cstdint>
inline int64_t fakeEspTimerMicros = 0;
inline int64_t esp_timer_get_time() { return fakeEspTimerMicros; }