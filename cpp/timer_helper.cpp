#include "timer_helper.h"

#include "timer.h"

#include <time.h>
#include <stdexcept>

UTimerNodeAllocator::UTimerNodeAllocator()
    : TimerAllocCalled_(0), TimerFreeCalled_(0), pFreeTimerHeadMem_(nullptr), TimerFreeCount_(0), UNIQS_TIMER_CACHE_MAX(4096), UNIQS_TIMER_CACHE_DELETE(2048) {}
UTimerNodeAllocator::~UTimerNodeAllocator() {
    UTimerNode* tmp = nullptr;
    while (pFreeTimerHeadMem_ != nullptr) {
        tmp = pFreeTimerHeadMem_;
        pFreeTimerHeadMem_ = pFreeTimerHeadMem_->pNext_;
        delete (tmp);
    }
}

UTimerNode* UTimerNodeAllocator::AllocObj() {
    ++TimerAllocCalled_;

    if (pFreeTimerHeadMem_ != nullptr) {
        --TimerFreeCount_;
        UTimerNode* pTimer = pFreeTimerHeadMem_;
        pFreeTimerHeadMem_ = pTimer->pNext_;
        pTimer->pNext_ = nullptr;
        return pTimer;
    }
    auto ret = new UTimerNode();
    return ret;
}
void UTimerNodeAllocator::FreeObj(UTimerNode* pTimer) {
    ++TimerFreeCalled_;

    ++TimerFreeCount_;
    if (pFreeTimerHeadMem_ == nullptr) {
        pFreeTimerHeadMem_ = pTimer;
        pFreeTimerHeadMem_->pNext_ = nullptr;
    } else {
        pTimer->pNext_ = pFreeTimerHeadMem_;
        pFreeTimerHeadMem_ = pTimer;
    }

    if (TimerFreeCount_ > UNIQS_TIMER_CACHE_MAX) {
        UTimerNode* pDelete = pFreeTimerHeadMem_;
        for (int i = 0; i < UNIQS_TIMER_CACHE_DELETE; ++i) {
            pFreeTimerHeadMem_ = pDelete->pNext_;

            // free memory
            delete pDelete;

            pDelete = pFreeTimerHeadMem_;
        }
        TimerFreeCount_ -= UNIQS_TIMER_CACHE_DELETE;
    }
}

#include <time.h>
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
#include <chrono>
#include <Windows.h>
#else
#include <sys/time.h>
#endif

TimerMsType UTimerGetCurrentTimeMS(void) {
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
#if 0
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    return (TimerMsType)duration_in_ms.count();
#endif
#if 1
    return UTimerGetTickCountMS();
#endif
#else
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
#endif
}

int64_t UTimerGetCurrentTimeUS(void) {
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch());
    return (TimerMsType)duration_in_ms.count();
#else
#if 1
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
#if 1
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
#endif
#if 0
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch());
    return (TimerMsType)duration_in_ms.count();
#endif
#endif
}

uint64_t UTimerGetTickCountMS() {
#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
    return ::GetTickCount64();
#else
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}

#include <iostream>
void UTimerOnError(const std::string& err) {
#if 0
    throw std::logic_error("UTimerOnError" + err);
#else
    std::cout << err << std::endl;
#endif
}
