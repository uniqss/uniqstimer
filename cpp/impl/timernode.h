#pragma once

#include "../timer_def.h"

class ListTimer
{
public:
	ListTimer* pPrev;
	ListTimer* pNext;
};

enum ETimerState
{
	ETimerStateInvalid = 0,
	ETimerStateRunning = 1,
	ETimerStateKilled = 2,
	ETimerStateReleased = 3,
};

class TimerNode
{
public:
	ListTimer ltTimer;
	TimerMsType uExpires;   // 定时器到期时间
	TimerMsType uPeriod;    // 定时器触发后，再次触发的间隔时长。如果为 0，表示该定时器为一次性的
	void (*timerFn)(TimerIdType, void*); // 定时器回调函数
	void* pParam;           // 回调函数的参数
	TimerIdType id;
	ETimerState state;
	bool FrameKilled;
};
