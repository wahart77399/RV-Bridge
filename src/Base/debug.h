// src/Base/debug.h
#pragma once

// -------------------------------------------------------
// Compile-time switch
//   -DRV_DEBUG=1   → prints enabled
//   -DRV_DEBUG=0   → all debug code disappears
// -------------------------------------------------------
#ifndef RV_DEBUG
#define RV_DEBUG 0
#endif

#if RV_DEBUG
  #include <Arduino.h>
  #define RV_PRINTF(...)        printf(__VA_ARGS__)
  #define RV_PRINT(x)           print(x)
  #define RV_PRINTLN(x)         println(x)
#else
  #define RV_PRINTF(...)        ((void)0)
  #define RV_PRINT(x)           ((void)0)
  #define RV_PRINTLN(x)         ((void)0)
#endif
