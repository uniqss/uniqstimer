#pragma once

#include <string>

struct UTimerNode;
class UTimerNodeAllocator {
   public:
    UTimerNodeAllocator();
    ~UTimerNodeAllocator();
    UTimerNode* AllocObj();
    void FreeObj(UTimerNode* pTimer);

    int TimerAllocCalled_;
    int TimerFreeCalled_;
    UTimerNode* pFreeTimerHeadMem_;
    int TimerFreeCount_;
    const int UNIQS_TIMER_CACHE_MAX;
    const int UNIQS_TIMER_CACHE_DELETE;
};

int64_t UTimerGetCurrentTimeMS(void);
int64_t UTimerGetCurrentTimeUS(void);

uint64_t UTimerGetTickCountMS();

void UTimerOnError(const std::string& err);
