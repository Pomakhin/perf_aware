#pragma once

#include <cstdint>
#include <intrin.h>
#include <windows.h>

static uint64_t GetOSTimerFreq(void) {
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    return Freq.QuadPart;
}

static uint64_t ReadOSTimer(void) {
    LARGE_INTEGER Value;
    QueryPerformanceCounter(&Value);
    return Value.QuadPart;
}

inline uint64_t ReadCPUTimer(void) {
    return __rdtsc();
}

static uint64_t approximated_cpu_freq_ = 1;
static uint64_t approximated_cpu_freq_ms_ = 1;

static double GetCPUTimeMs() {
    return static_cast<double>(ReadCPUTimer()) / static_cast<double>(approximated_cpu_freq_ms_);
}

static uint64_t ApproximateCPUTimerFreq() {
    uint64_t os_counter_start = ReadOSTimer();
    uint64_t os_timer_freq = GetOSTimerFreq();
    uint64_t cpu_counter_start = ReadCPUTimer();
    float interval = 0.2f;
    uint64_t os_ticks_to_wait = os_timer_freq * interval;
    while (ReadOSTimer() - os_counter_start < os_ticks_to_wait) {
    }
    approximated_cpu_freq_ = (ReadCPUTimer() - cpu_counter_start) / interval;
    approximated_cpu_freq_ms_ = approximated_cpu_freq_ / 1000;
    return approximated_cpu_freq_;
}