#pragma once

#include <mutex>
#include <thread>
#include<atomic>
#include<stdint.h>

#define TimerMsType uint64_t

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define TIME_AFTER(a,b) ((long)(b) - (long)(a) < 0)
#define TIME_BEFORE(a,b) TIME_AFTER(b,a)
#define TIME_AFTER_EQ(a,b) ((long)(a) - (long)(b) >= 0)
#define TIME_BEFORE_EQ(a,b) TIME_AFTER_EQ(b,a)

class ListTimer
{
public:
	ListTimer* pPrev;
	ListTimer* pNext;
};

class TimerNode
{
public:
	ListTimer ltTimer;
	TimerMsType uExpires;   // 定时器到期时间
	TimerMsType uPeriod;    // 定时器触发后，再次触发的间隔时长。如果为 0，表示该定时器为一次性的
	void (*timerFn)(void*); // 定时器回调函数
	void* pParam;           // 回调函数的参数
};

class TimerManager
{
public:
	std::mutex lock;
	std::thread* thread;
	std::atomic<bool> threadWorking;
	std::atomic<bool> threadWaitingTerminateOK;

	TimerMsType currentTimeMS;          // 基准时间(当前时间)，单位：毫秒
	ListTimer arrListTimer1[TVR_SIZE];  // 1 级时间轮。在这里表示存储未来的 0 ~ 255 毫秒的计时器。tick 的粒度为 1 毫秒
	ListTimer arrListTimer2[TVN_SIZE];  // 2 级时间轮。存储未来的 256 ~ 256*64-1 毫秒的计时器。tick 的粒度为 256 毫秒
	ListTimer arrListTimer3[TVN_SIZE];  // 3 级时间轮。存储未来的 256*64 ~ 256*64*64-1 毫秒的计时器。tick 的粒度为 256*64 毫秒
	ListTimer arrListTimer4[TVN_SIZE];  // 4 级时间轮。存储未来的 256*64*64 ~ 256*64*64*64-1 毫秒的计时器。tick 的粒度为 256*64*64 毫秒
	ListTimer arrListTimer5[TVN_SIZE];  // 5 级时间轮。存储未来的 256*64*64*64 ~ 256*64*64*64*64-1 毫秒的计时器。tick 的粒度为 256*64*64 毫秒
	ListTimer arrListTimer6[TVN_SIZE];  // 5 级时间轮。存储未来的 256*64*64*64*64 ~ 256*64*64*64*64*64-1 毫秒的计时器。tick 的粒度为 256*64*64*64 毫秒
};

int64_t UTimerGetCurrentTimeMS(void);

TimerManager* CreateTimerManager(void);

void DestroyTimerManager(TimerManager* lpTimerManager);

// 创建一个定时器  uDueTime 首次触发的超时时间间隔  uPeriod 定时器循环周期，若为0，则该定时器只运行一次
TimerNode* CreateTimer(TimerManager* lpTimerManager, void (*timerFn)(void*), void* pParam, TimerMsType uDueTime, TimerMsType uPeriod);

// 删除定时器
int32_t DeleteTimer(TimerManager* lpTimerManager, TimerNode* lpTimer);

