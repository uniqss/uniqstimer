#pragma once

#include "timer.h"
#include "timerIII.h"
#include <string>

TimerNode* AllocObj();
void FreeObj(TimerNode* pTimer);

TimerNodeIII* AllocObjIII();
void FreeObjIII(TimerNodeIII* pTimer);

int64_t UTimerGetCurrentTimeMS(void);
void OnTimerError(const std::string& err);
