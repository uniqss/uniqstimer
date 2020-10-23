#pragma once

#include <mutex>
#include <thread>
#include<atomic>
#include<unordered_map>
#include<unordered_set>
#include<stdint.h>


#define TimerMsType int64_t
#define TimerIdType int64_t

#define TIMER_BITS_PER_WHEEL 8
#define TIMER_SLOT_COUNT_PER_WHEEL 1 << TIMER_BITS_PER_WHEEL
#define TIMER_WHEEL_COUNT 5
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

class TimerNode
{
public:
	TimerNode* pNext;
	TimerIdType qwTimerId;
	TimerMsType qwExpires; // 
	TimerMsType qwPeriod;
	void (*timerFn)(TimerIdType, void*, TimerMsType currTimeMS);
	void* pParam;
	bool bRunning;
};

class TimerManager
{
public:
	TimerMsType qwCurrentTimeMS; // current time ms
	std::unordered_map<TimerIdType, TimerNode*> pTimers;
	TimerNode* arrListTimer[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
public:
	void Run();
};

int64_t UTimerGetCurrentTimeMS(void);
void OnTimerError(const std::string& err);

TimerManager* CreateTimerManager(void);

void DestroyTimerManager(TimerManager* pTimerManager);

// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void (*timerFn)(TimerIdType, void*, TimerMsType currTimeMS), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod);

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId);
