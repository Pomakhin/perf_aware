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

static uint64_t ApproximateCPUTimerFreq() {
    uint64_t os_counter_start = ReadOSTimer();
    uint64_t os_timer_freq = GetOSTimerFreq();
    uint64_t cpu_counter_start = ReadCPUTimer();
    while (ReadOSTimer() - os_counter_start < os_timer_freq) {
    }
    return ReadCPUTimer() - cpu_counter_start;
}