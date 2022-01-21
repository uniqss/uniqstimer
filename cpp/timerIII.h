#pragma once

#include <unordered_map>
#include <stdint.h>

#include "timer_def.h"

#define TimerMsTypeIII int64_t

#define TIMER_BITS_PER_WHEELIII 10
#define TIMER_SLOT_COUNT_PER_WHEELIII 1 << TIMER_BITS_PER_WHEELIII
#define TIMER_WHEEL_COUNTIII 4
#define TIMER_MASKIII ((1 << TIMER_BITS_PER_WHEELIII) - 1)

class TimerNodeIII {
   public:
    TimerNodeIII* pNext;
    TimerIdType qwTimerId;
    TimerMsTypeIII qwExpires;  //
    TimerMsTypeIII qwPeriod;
    void (*timerFn)(TimerIdType, void*);
    void* pParam;
    bool bRunning;
};

class TimerManagerIII {
   public:
    TimerManagerIII(TimerMsTypeIII TIMER_MS_COUNTIII = 100);
    ~TimerManagerIII();

    // qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
    bool CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsTypeIII qwDueTime, TimerMsTypeIII qwPeriod);
    bool KillTimer(TimerIdType timerId);
    void KillAllTimers();

   public:
    TimerMsTypeIII qwCurrentTimeMS_;  // current time ms / 100
    const TimerMsTypeIII qwTickWheelMS_;
    std::unordered_map<TimerIdType, TimerNodeIII*> mapTimers_;
    TimerNodeIII* arrListTimerHead_[TIMER_WHEEL_COUNTIII][TIMER_SLOT_COUNT_PER_WHEELIII];
    TimerNodeIII* arrListTimerTail_[TIMER_WHEEL_COUNTIII][TIMER_SLOT_COUNT_PER_WHEELIII];

   public:
    void Run();
};
