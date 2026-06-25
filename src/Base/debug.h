#ifndef DEBUGGING_H
#define DEBUGGING_H
#include "Arduino.h"
#include "HomeSpan.h"
#include <iostream>
#include <sstream>  // For std::ostringstream
#include <iomanip>

typedef enum {
    VERBOSE_LOG_LEVEL = 2,
    NOT_SO_VERBOSE_LOG_LEVEL = 1,
    MIN_LOG_LEVEL = 0,
    NO_LOG_LEVEL = 100
} RVC_LOG_LEVEL;
constexpr boolean DEBUGGING = true;

constexpr RVC_LOG_LEVEL RVC_Log_Level = MIN_LOG_LEVEL;

// std::ostringstream oss;

void LOGIT(RVC_LOG_LEVEL level, std::ostringstream& s); 

#endif