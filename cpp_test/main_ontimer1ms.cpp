﻿#include "main.h"

#include "timer_helper.h"

void OnTimerPressureTest(TimerIdType timerId, void* pParam) {
    // const char* pszStr = (const char*)pParam;
    ++FrameOnTimerCalled;
    ++OnTimerCount;
    // int64_t currTimeMS = UTimerGetCurrentTimeMS();

    // printf("OnTimerIII timerId:%lld pszStr:%s currMS:%lld\n", timerId, pszStr, currTimeMS);
}
