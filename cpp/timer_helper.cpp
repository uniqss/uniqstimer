#include "timer_helper.h"

#include <time.h>
#include <stdexcept>

int UniqsTimerAllocCalled = 0;
int UniqsTimerFreeCalled = 0;

TimerNode* __pFreeTimerHeadMem;
int UniqsTimerFreeCount = 0;
const int UNIQS_TIMER_CACHE_MAX = 4096;
const int UNIQS_TIMER_CACHE_DELETE = UNIQS_TIMER_CACHE_MAX / 2;

TimerNodeIII* __pFreeTimerHeadMemIII;
int UniqsTimerFreeCountIII = 0;
const int UNIQS_TIMER_CACHE_MAXIII = 4096;
const int UNIQS_TIMER_CACHE_DELETEIII = UNIQS_TIMER_CACHE_MAXIII / 2;

TimerNode* AllocObj() {
    ++UniqsTimerAllocCalled;

    if (__pFreeTimerHeadMem != nullptr) {
        UniqsTimerFreeCount--;
        TimerNode* pTimer = __pFreeTimerHeadMem;
        __pFreeTimerHeadMem = pTimer->pNext;
        pTimer->pNext = nullptr;
        return pTimer;
    }
    auto ret = new TimerNode();
    return ret;
}
void FreeObj(TimerNode* pTimer) {
    ++UniqsTimerFreeCalled;

    ++UniqsTimerFreeCount;
    if (__pFreeTimerHeadMem == nullptr) {
        __pFreeTimerHeadMem = pTimer;
        __pFreeTimerHeadMem->pNext = nullptr;
    } else {
        pTimer->pNext = __pFreeTimerHeadMem;
        __pFreeTimerHeadMem = pTimer;
    }

    if (UniqsTimerFreeCount > UNIQS_TIMER_CACHE_MAX) {
        TimerNode* pDelete = __pFreeTimerHeadMem;
        for (int i = 0; i < UNIQS_TIMER_CACHE_DELETE; ++i) {
            __pFreeTimerHeadMem = pDelete->pNext;

            // free memory
            delete pDelete;

            pDelete = __pFreeTimerHeadMem;
        }
        UniqsTimerFreeCount -= UNIQS_TIMER_CACHE_DELETE;
    }
}

TimerNodeIII* AllocObjIII() {
    if (__pFreeTimerHeadMemIII != nullptr) {
        UniqsTimerFreeCountIII--;
        TimerNodeIII* pTimer = __pFreeTimerHeadMemIII;
        __pFreeTimerHeadMemIII = pTimer->pNext;
        pTimer->pNext = nullptr;
        return pTimer;
    }
    auto ret = new TimerNodeIII();
    return ret;
}
void FreeObjIII(TimerNodeIII* pTimer) {
    ++UniqsTimerFreeCountIII;
    if (__pFreeTimerHeadMemIII == nullptr) {
        __pFreeTimerHeadMemIII = pTimer;
        __pFreeTimerHeadMemIII->pNext = nullptr;
    } else {
        pTimer->pNext = __pFreeTimerHeadMemIII;
        __pFreeTimerHeadMemIII = pTimer;
    }


    if (UniqsTimerFreeCountIII > UNIQS_TIMER_CACHE_MAXIII) {
        TimerNodeIII* pDelete = __pFreeTimerHeadMemIII;
        for (int i = 0; i < UNIQS_TIMER_CACHE_DELETEIII; ++i) {
            __pFreeTimerHeadMemIII = pDelete->pNext;

            // free memory
            delete pDelete;

            pDelete = __pFreeTimerHeadMemIII;
        }
        UniqsTimerFreeCountIII -= UNIQS_TIMER_CACHE_DELETEIII;
    }
}
#include <time.h>
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
#include <windows.h>
#else
#include <sys/time.h>
#endif
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
int gettimeofday(struct timeval* tp, void* tzp) {
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif

TimerMsType UTimerGetCurrentTimeMS(void) {
#if 1
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
#if 0
    unsigned int lo, hi;

    // RDTSC copies contents of 64-bit TSC into EDX:EAX
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return (unsigned long long)hi << 32 | lo;
#endif
#if 0
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    return (TimerMsType)duration_in_ms.count();
#endif
}

int64_t UTimerGetCurrentTimeUS(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

#include <iostream>
void OnTimerError(const std::string& err) {
#if 0
    throw std::logic_error("OnTimerError" + err);
#else
    std::cout << err << std::endl;
#endif
}
