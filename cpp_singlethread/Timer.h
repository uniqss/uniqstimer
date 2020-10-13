#pragma once

#include<stdint.h>

#include "impl/simplepool.h"
#include "impl/timernode.h"
#include "timer_def.h"

class TimerManager
{
public:
	TimerMsType currentTimeMS;
	ListTimer arrListTimer1[TVR_SIZE];
	ListTimer arrListTimer2[TVN_SIZE];
	ListTimer arrListTimer3[TVN_SIZE];
	ListTimer arrListTimer4[TVN_SIZE];
	ListTimer arrListTimer5[TVN_SIZE];
	ListTimer arrListTimer6[TVN_SIZE];
	
	SimplePool<TimerNode> timerPool;
	void Run();
};

uint64_t UTimerGetCurrentTimeMS(void);

TimerManager* CreateTimerManager();

void DestroyTimerManager(TimerManager* pTimerManager);

bool CreateTimer(TimerIdType timerId, TimerManager* pTimerManager, void (*OnTimer)(TimerIdType, void*), void* pParam, TimerMsType uDueTime, TimerMsType uPeriod);

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId);

