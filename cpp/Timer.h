#pragma once

#include <mutex>
#include <thread>
#include<atomic>
#include<stdint.h>
#include<unordered_set>

#include "impl/simplepool.h"
#include "impl/timernode.h"
#include "timer_def.h"

class TimerManager
{
public:
	bool internalThread;
	std::mutex lock;
	std::thread* thread;
	std::atomic<bool> threadWorking;
	std::atomic<bool> threadWaitingTerminateOK;

	TimerMsType currentTimeMS;
	ListTimer arrListTimer1[TVR_SIZE];
	ListTimer arrListTimer2[TVN_SIZE];
	ListTimer arrListTimer3[TVN_SIZE];
	ListTimer arrListTimer4[TVN_SIZE];
	ListTimer arrListTimer5[TVN_SIZE];
	ListTimer arrListTimer6[TVN_SIZE];

	std::unordered_set<TimerIdType> pendingReleaseTimers;
	
	SimplePool<TimerNode> timerPool;
};

uint64_t UTimerGetCurrentTimeMS(void);

TimerManager* CreateTimerManager(bool internalThraed);

void DestroyTimerManager(TimerManager* lpTimerManager);

bool CreateTimer(TimerIdType timerId, TimerManager* lpTimerManager, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType uDueTime, TimerMsType uPeriod);

bool KillTimer(TimerManager* lpTimerManager, TimerIdType timerId);

