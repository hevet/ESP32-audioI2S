#pragma once

#include "Arduino.h"
#include "../psram_unique_ptr.hpp"
#include "libmad-0.15.0b/defines.h"
#include "libmad-0.15.0b/structures.h"
#include "libmad-0.15.0b/tables.hpp"
#include "libmad-0.15.0b/libmad.h"


// globale oder static Variablen
extern ps_ptr<mad_stream_t>stream;
extern ps_ptr<mad_frame_t> frame;
extern ps_ptr<mad_synth_t> synth;

bool        MP3Decoder_AllocateBuffers();
void        MP3Decoder_FreeBuffers();
void        MP3Decoder_ClearBuffer();
int32_t     MP3Decode(uint8_t* data, int32_t* bytesLeft, int16_t* outSamples);
uint32_t    MP3GetOutputSamps();
int32_t     MP3GetSampRate();
int32_t     MP3GetChannels();
uint8_t     MP3GetBitsPerSample();
bool        MP3Decoder_IsInit();
const char* MP3GetMPEGVersion();
const char* MP3GetLayer();
uint32_t    MP3GetBitrate();
uint32_t    MP3GetAudioFileDuration();
int32_t     MP3FindSyncWord(uint8_t* buf, int32_t nBytes);

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    // 📌📌📌  L O G G I N G   📌📌📌

template <typename... Args>
void MP3_LOG_IMPL(uint8_t level, const char* path, int line, const char* fmt, Args&&... args) {
    extern __attribute__((weak)) void audio_info(const char*);
    #define ANSI_ESC_RESET          "\033[0m"
    #define ANSI_ESC_BLACK          "\033[30m"
    #define ANSI_ESC_RED            "\033[31m"
    #define ANSI_ESC_GREEN          "\033[32m"
    #define ANSI_ESC_YELLOW         "\033[33m"
    #define ANSI_ESC_BLUE           "\033[34m"
    #define ANSI_ESC_MAGENTA        "\033[35m"
    #define ANSI_ESC_CYAN           "\033[36m"
    #define ANSI_ESC_WHITE          "\033[37m"

    ps_ptr<char> result;
    ps_ptr<char> file;

    file.copy_from(path);
    while(file.contains("/")){
        file.remove_before('/', false);
    }

    // First run: determine size
    int len = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
    if (len <= 0) return;

    result.alloc(len + 1, "result");
    char* dst = result.get();
    if (!dst) return;
    std::snprintf(dst, len + 1, fmt, std::forward<Args>(args)...);

    // build a final string with file/line prefix
    ps_ptr<char> final;
    int total_len = std::snprintf(nullptr, 0, "%s:%d:" ANSI_ESC_RED " %s" ANSI_ESC_RESET, file.c_get(), line, dst);
    if (total_len <= 0) return;
    final.alloc(total_len + 1, "final");
    final.clear();
    char* dest = final.get();
    if (!dest) return;  // Or error treatment
    if(audio_info){
        if     (level == 1 && CORE_DEBUG_LEVEL >= 1) snprintf(dest, total_len + 1, "%s:%d:" ANSI_ESC_RED " %s" ANSI_ESC_RESET, file.c_get(), line, dst);
        else if(level == 2 && CORE_DEBUG_LEVEL >= 2) snprintf(dest, total_len + 1, "%s:%d:" ANSI_ESC_YELLOW " %s" ANSI_ESC_RESET, file.c_get(), line, dst);
        else if(level == 3 && CORE_DEBUG_LEVEL >= 3) snprintf(dest, total_len + 1, "%s:%d:" ANSI_ESC_GREEN " %s" ANSI_ESC_RESET, file.c_get(), line, dst);
        else if(level == 4 && CORE_DEBUG_LEVEL >= 4) snprintf(dest, total_len + 1, "%s:%d:" ANSI_ESC_CYAN " %s" ANSI_ESC_RESET, file.c_get(), line, dst);  // debug
        else              if( CORE_DEBUG_LEVEL >= 5) snprintf(dest, total_len + 1, "%s:%d:" ANSI_ESC_WHITE " %s" ANSI_ESC_RESET, file.c_get(), line, dst); // verbose
        if(final.strlen() > 0)  audio_info(final.get());
    }
    else{
        std::snprintf(dest, total_len + 1, "%s:%d: %s", file.c_get(), line, dst);
        if     (level == 1) log_e("%s", final.c_get());
        else if(level == 2) log_w("%s", final.c_get());
        else if(level == 3) log_i("%s", final.c_get());
        else if(level == 4) log_d("%s", final.c_get());
        else                log_v("%s", final.c_get());
    }
    final.reset();
    result.reset();
}

// Macro for comfortable calls
#define MP3_ERROR(fmt, ...) MP3_LOG_IMPL(1, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MP3_WARN(fmt, ...)  MP3_LOG_IMPL(2, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MP3_INFO(fmt, ...)  MP3_LOG_IMPL(3, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MP3_DEBUG(fmt, ...) MP3_LOG_IMPL(4, __FILE__, __LINE__, fmt, ##__VA_ARGS__)