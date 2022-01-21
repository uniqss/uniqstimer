#pragma once

#include "timer.h"
#include <string>

TimerNode* AllocObj();
void FreeObj(TimerNode* pTimer);

int64_t UTimerGetCurrentTimeMS(void);
int64_t UTimerGetCurrentTimeUS(void);
void OnTimerError(const std::string& err);
