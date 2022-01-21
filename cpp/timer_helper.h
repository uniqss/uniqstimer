#pragma once

#include <string>

class TimerNode;
class TimerNodeAllocator {
   public:
    TimerNodeAllocator();
    ~TimerNodeAllocator();
    TimerNode* AllocObj();
    void FreeObj(TimerNode* pTimer);

    int TimerAllocCalled_;
    int TimerFreeCalled_;
    TimerNode* pFreeTimerHeadMem_;
    int TimerFreeCount_;
    const int UNIQS_TIMER_CACHE_MAX;
    const int UNIQS_TIMER_CACHE_DELETE;
};

int64_t UTimerGetCurrentTimeMS(void);
int64_t UTimerGetCurrentTimeUS(void);

uint32_t GetTickCount32MS();

void OnTimerError(const std::string& err);
