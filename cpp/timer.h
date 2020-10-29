#pragma once

#include<unordered_map>
#include<stdint.h>

#include "timer_def.h"


#define TimerMsType int64_t

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
	void (*timerFn)(TimerIdType, void*);
	void* pParam;
	bool bRunning;
};

class TimerManager
{
public:
	TimerManager();
	~TimerManager();

	// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
	bool CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod);
	bool KillTimer(TimerIdType timerId);
	void KillAllTimers();

public:
	TimerMsType qwCurrentTimeMS; // current time ms
	std::unordered_map<TimerIdType, TimerNode*> pTimers;
	TimerNode* arrListTimerHead[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
	TimerNode* arrListTimerTail[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
public:
	void Run();
};

