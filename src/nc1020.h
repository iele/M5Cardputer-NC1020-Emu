#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <string>
#include <cstdint>
#endif

namespace wqx
{
    struct WqxRom
    {
        std::string romPath;
        std::string norFlashPath;
        std::string statesPath;
        std::string romTempPath;
        std::string norTempPath;
    };
    typedef struct WqxRom WqxRom;
    extern void Initialize(WqxRom);
    extern void Reset();
    extern void SetKey(uint8_t, bool);
    extern void RunTimeSlice(uint32_t, bool);
    extern bool CopyLcdBuffer(uint8_t *);
    extern void LoadNC1020();
    extern void SaveNC1020();

}
