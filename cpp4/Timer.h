#pragma once

#include <mutex>
#include <thread>
#include<atomic>
#include<unordered_map>
#include<unordered_set>
#include<stdint.h>

#if 1
#define UNIQS_LOG_EVERYTHING
#endif

#ifdef UNIQS_LOG_EVERYTHING
#include "glog_helper.h"
#endif


#define TimerMsType int64_t
#define TimerIdType int64_t

#define TIMER_BITS_PER_WHEEL 8
#define TIMER_COUNT_PER_WHEEL 1 << 8
#define TIMER_WHEEL_COUNT 5
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

const int ETimerState_Killed = 0;
const int ETimerState_Running = 1;

#if 0
#define UNIQS_DEBUG_TIMER
#endif

#ifdef UNIQS_DEBUG_TIMER
extern TimerMsType DebugDiffTimeMs;
#endif

class TimerNode
{
public:
	TimerNode* pPrev;
	TimerNode* pNext;
	TimerIdType qwTimerId;
	TimerMsType qwExpires; // 
	TimerMsType qwPeriod;
	void (*timerFn)(TimerIdType, void*);
	void* pParam;
	int state;
};

class TimerManager
{
public:
	TimerMsType qwCurrentTimeMS; // current time ms
	std::unordered_map<TimerIdType, TimerNode*> pTimers;
	std::list<TimerNode*> arrListTimer[TIMER_WHEEL_COUNT][TIMER_COUNT_PER_WHEEL];
public:
	void Run();
};

int64_t UTimerGetCurrentTimeMS(void);
void OnTimerError(const std::string& err);

TimerManager* CreateTimerManager(void);

void DestroyTimerManager(TimerManager* pTimerManager);

// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod);

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId);
