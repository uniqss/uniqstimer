#pragma once

#include<unordered_map>
#include<stdint.h>


#define TimerMsTypeIII int
#define TimerIdTypeIII int

#define TIMER_BITS_PER_WHEELIII 6
#define TIMER_SLOT_COUNT_PER_WHEELIII 1 << TIMER_BITS_PER_WHEELIII
#define TIMER_WHEEL_COUNTIII 5
#define TIMER_MASKIII ((1 << TIMER_BITS_PER_WHEELIII) - 1)
#define TIMER_MS_COUNTIII 100

class TimerNodeIII
{
public:
	TimerNodeIII* pNext;
	TimerIdTypeIII qwTimerId;
	TimerMsTypeIII qwExpires; // 
	TimerMsTypeIII qwPeriod;
	void (*timerFn)(TimerIdTypeIII, void*);
	void* pParam;
	bool bRunning;
};

class TimerManagerIII
{
public:
	TimerManagerIII();
	~TimerManagerIII();

	// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
	bool CreateTimer(TimerIdTypeIII timerId, void (*timerFn)(TimerIdTypeIII, void*), void* pParam, TimerMsTypeIII qwDueTime, TimerMsTypeIII qwPeriod);
	bool KillTimer(TimerIdTypeIII timerId);

public:
	TimerMsTypeIII qwCurrentTimeMS; // current time ms / 100
	std::unordered_map<TimerIdTypeIII, TimerNodeIII*> pTimers;
	TimerNodeIII* arrListTimerHead[TIMER_WHEEL_COUNTIII][TIMER_SLOT_COUNT_PER_WHEELIII];
	TimerNodeIII* arrListTimerTail[TIMER_WHEEL_COUNTIII][TIMER_SLOT_COUNT_PER_WHEELIII];
public:
	void Run();
};

