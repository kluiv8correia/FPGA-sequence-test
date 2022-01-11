#ifndef LOGGER_H
#define LOGGER_H

// INCLUDES -----------------
#include <Arduino.h>
// --------------------------


#define LOG(TYPE, MESSAGE, ...) { Serial.printf("[%s]: ", TYPE); Serial.printf(MESSAGE, ##__VA_ARGS__); Serial.println(); }
#define SERIAL(MESSAGE) { Serial.print(MESSAGE); }
#define LOG_ERROR(MESSAGE, ...) { LOG("ERROR", MESSAGE, ##__VA_ARGS__); }
#define LOG_INTERRUPT(MESSAGE, ...) { LOG("INTERRUPT", MESSAGE, ##__VA_ARGS__); }
#define LOG_REPORT(MESSAGE, ...) { LOG("REPORT", MESSAGE, ##__VA_ARGS__); }


#endif