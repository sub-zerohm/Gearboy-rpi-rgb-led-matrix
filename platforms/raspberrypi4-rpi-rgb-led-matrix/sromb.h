#ifndef SROMB_H
#define SROMB_H

#include <string.h>

#define DEBUG_GEARBOY

#ifdef DEBUG_GEARBOY
    #ifdef __ANDROID__
        #include <android/log.h>
        #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "GEARBOY", __VA_ARGS__);
    #endif
#define Log(msg, ...) (Log_func(msg, ##__VA_ARGS__))
#else
#define Log(msg, ...)
#endif
#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144

struct palette_color
{
    int red;
    int green;
    int blue;
    int alpha;
};

typedef uint8_t u8;
inline void Log_func(const char* const msg, ...)
{
    static int count = 1;
    char szBuf[512];

    va_list args;
    va_start(args, msg);
    vsprintf(szBuf, msg, args);
    va_end(args);

    printf("%d: %s\n", count, szBuf);

    count++;
}

struct GB_Color
{
#if defined(__LIBRETRO__)
    #if defined(IS_LITTLE_ENDIAN)
    u8 blue;
    u8 green;
    u8 red;
    u8 alpha;
    #elif defined(IS_BIG_ENDIAN)
    u8 alpha;
    u8 red;
    u8 green;
    u8 blue;
    #endif
#else
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
#endif
};

void runRom(std::string romFile, bool isAutoStart = false);
void end(void);

#endif