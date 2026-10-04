#pragma once
inline int fakeResetReason = 7;
inline int esp_reset_reason() { return fakeResetReason; }