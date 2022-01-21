#pragma once

#include <unordered_map>
#include <stdint.h>

#include "timer_def.h"

#define TimerMsType int64_t

#define TIMER_BITS_PER_WHEEL 10
#define TIMER_SLOT_COUNT_PER_WHEEL 1 << TIMER_BITS_PER_WHEEL
#define TIMER_WHEEL_COUNT 4
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

class TimerNode {
   public:
    TimerNode* pNext;
    TimerIdType qwTimerId;
    TimerMsType qwExpires;  //
    TimerMsType qwPeriod;
    void (*timerFn)(TimerIdType, void*);
    void* pParam;
    bool bRunning;
};

class TimerManager {
   public:
    TimerManager();
    ~TimerManager();

    // qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
    bool CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod);
    bool KillTimer(TimerIdType timerId);
    void KillAllTimers();

   public:
    TimerMsType qwCurrentTimeMS_;  // current time ms
    std::unordered_map<TimerIdType, TimerNode*> mapTimers_;
    TimerNode* arrListTimerHead_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    TimerNode* arrListTimerTail_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];

   public:
    void Run();
};
