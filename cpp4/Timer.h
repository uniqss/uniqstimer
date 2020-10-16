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
#define TIMER_COUNT_PER_WHEEL 1 << 8
#define TIMER_WHEEL_COUNT 5
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

const int ETimerState_Invalid = 0 << 0;
const int ETimerState_Running = 1 << 1;
const int ETimerState_Killed = 1 << 2;
const int ETimerState_FrameChanged = 1 << 3;

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
	std::unordered_set<TimerIdType> pPendingDeleteTimers;
	TimerNode arrListTimer[TIMER_WHEEL_COUNT][TIMER_COUNT_PER_WHEEL];
public:
	void Run();
private:
	void pendingFree();
};

int64_t UTimerGetCurrentTimeMS(void);

TimerManager* CreateTimerManager(void);

void DestroyTimerManager(TimerManager* pTimerManager);

// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod);

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId);
