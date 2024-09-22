#pragma once

#include <string>
#include <stdint.h>

struct UTimerNode;
class UTimerNodeAllocator {
   public:
    UTimerNodeAllocator();
    ~UTimerNodeAllocator();
    UTimerNode* AllocObj();
    void FreeObj(UTimerNode* pTimer);

    int alloc_called_;
    int free_called_;
    UTimerNode* free_timer_head_;
    int free_count_;
    const int UNIQS_TIMER_CACHE_MAX;
    const int UNIQS_TIMER_CACHE_DELETE;
};

int64_t UTimerGetCurrentTimeMS(void);
int64_t UTimerGetCurrentTimeUS(void);

uint64_t UTimerGetTickCountMS();

void UTimerOnError(const std::string& err);
