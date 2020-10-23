#include "Timer.h"

#include "timer_mem.h"

#include <stddef.h>
#include <stdlib.h>
#include<chrono>
#include <cassert>
#include <time.h>


TimerMsType UTimerGetCurrentTimeMS(void)
{
	return clock();
}

void OnTimerError(const std::string& err)
{
	throw std::logic_error("OnTimerError" + err);
}

enum EAddTimerSource
{
	EAddTimerSource_Create_NewAlloc,
	EAddTimerSource_Cascade,
	EAddTimerSource_TimeoutReadd,
};
const char* pszAddTimerSource[] = {
	"EAddTimerSource_Create_NewAlloc",
	"EAddTimerSource_Cascade",
	"EAddTimerSource_TimeoutReadd",
};

static void AddTimer(TimerManager* pTimerManager, TimerNode* pTimer, TimerMsType fromWheelIdx, TimerMsType fromSlotIdx, EAddTimerSource source)
{
	TimerMsType wheelIdx = 0;
	TimerMsType slotIdx = 0;

	TimerMsType qwExpires = pTimer->qwExpires;
	TimerMsType qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS;

	if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (TIMER_WHEEL_COUNT)))
	{
		for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT; i++)
		{
			if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (i + 1)))
			{
				slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * i)) & TIMER_MASK;
				wheelIdx = i;
				break;
			}
		}
	}
	else if (qwDueTime < 0)
	{
		wheelIdx = 0;
		slotIdx = pTimerManager->qwCurrentTimeMS & TIMER_MASK;
	}
	else
	{
		OnTimerError("AddTimer this should not happen");
		return;
	}

	if (pTimerManager->arrListTimerTail[wheelIdx][slotIdx] == nullptr)
	{
		pTimerManager->arrListTimerHead[wheelIdx][slotIdx] = pTimer;
		pTimerManager->arrListTimerTail[wheelIdx][slotIdx] = pTimer;
		pTimer->pNext = nullptr;
	}
	else
	{
		pTimerManager->arrListTimerTail[wheelIdx][slotIdx]->pNext = pTimer;
		pTimer->pNext = nullptr;
		pTimerManager->arrListTimerTail[wheelIdx][slotIdx] = pTimer;
	}
}

// 循环该slot里的所有节点，重新AddTimer到管理器中
static void CascadeTimer(TimerManager* pTimerManager, TimerMsType wheelIdx, TimerMsType slotIdx)
{
	TimerNode* pTimer = pTimerManager->arrListTimerHead[wheelIdx][slotIdx];
	TimerNode* pNext = nullptr;
	for (;pTimer != nullptr;pTimer = pNext)
	{
		pNext = pTimer->pNext;

		if (pTimer->bRunning)
		{
			AddTimer(pTimerManager, pTimer, wheelIdx, slotIdx, EAddTimerSource_Cascade);
		}
		else
		{
			FreeObj(pTimer);
		}
	}
	pTimerManager->arrListTimerHead[wheelIdx][slotIdx] = nullptr;
	pTimerManager->arrListTimerTail[wheelIdx][slotIdx] = nullptr;
}

void TimerManager::Run()
{
	TimerMsType idxExecutingSlotIdx = 0, idxNextWheelSlotIdx = 0;

	auto currTimeMS = UTimerGetCurrentTimeMS();
	TimerIdType timerId = 0;
	while (currTimeMS >= this->qwCurrentTimeMS)
	{
		idxExecutingSlotIdx = this->qwCurrentTimeMS & TIMER_MASK;

		idxNextWheelSlotIdx = idxExecutingSlotIdx;
		for (TimerMsType i = 0; i < 4 && idxNextWheelSlotIdx == 0; i++)
		{
			idxNextWheelSlotIdx = (this->qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
			CascadeTimer(this, i + 1, idxNextWheelSlotIdx);
		}

		TimerNode* pTimer = arrListTimerHead[0][idxExecutingSlotIdx];
		TimerNode* pNext = nullptr;
		for (;pTimer != nullptr;pTimer = pNext)
		{
			pNext = pTimer->pNext;

			timerId = pTimer->qwTimerId;
			if (pTimer->bRunning)
			{
				pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam, this->qwCurrentTimeMS);
				if (pTimer->qwPeriod != 0)
				{
					pTimer->qwExpires = this->qwCurrentTimeMS + pTimer->qwPeriod;
					AddTimer(this, pTimer, 0, idxExecutingSlotIdx, EAddTimerSource_TimeoutReadd);
				}
				else
				{
					FreeObj(pTimer);
					pTimers.erase(timerId);
				}
			}
			else
			{
				FreeObj(pTimer);
			}
		}
		arrListTimerHead[0][idxExecutingSlotIdx] = nullptr;
		arrListTimerTail[0][idxExecutingSlotIdx] = nullptr;

		this->qwCurrentTimeMS++;
	}
}

TimerManager::TimerManager()
{
	for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++)
	{
		for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++)
		{
			this->arrListTimerHead[wheelIdx][slotIdx] = nullptr;
			this->arrListTimerTail[wheelIdx][slotIdx] = nullptr;
		}
	}

	this->qwCurrentTimeMS = UTimerGetCurrentTimeMS();
}

TimerManager::~TimerManager()
{
	for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++)
	{
		for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++)
		{
			TimerNode* pTimer = this->arrListTimerHead[wheelIdx][slotIdx];
			TimerNode* pNext = nullptr;
			for (;pTimer != nullptr;pTimer = pNext)
			{
				pNext = pTimer->pNext;

				FreeObj(pTimer);
			}
		}
	}
}

bool TimerManager::CreateTimer(TimerIdType timerId, void(*timerFn)(TimerIdType, void*, TimerMsType currTimeMS), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod)
{
	if (NULL == timerFn)
		return false;

	// 两者都为0,无意义
	if (qwDueTime == 0 && qwPeriod == 0)
	{
		return false;
	}

	// 如果创建重复性定时器，又设置触发时间为0,就直接把时间设置为重复时间
	if (qwDueTime == 0)
	{
		return false;
	}

	auto it = this->pTimers.find(timerId);
	if (it != this->pTimers.end())
	{
		return false;
	}

	TimerNode* pTimer = AllocObj();
	if (pTimer == nullptr)
	{
		OnTimerError("CreateTimer AllocObj failed.");
		return false;
	}

	pTimer->qwPeriod = qwPeriod;
	pTimer->timerFn = timerFn;
	pTimer->pParam = pParam;
	pTimer->qwTimerId = timerId;

	pTimer->bRunning = true;

	pTimer->qwExpires = this->qwCurrentTimeMS + qwDueTime;
	AddTimer(this, pTimer, 0, 0, EAddTimerSource_Create_NewAlloc);
	this->pTimers[timerId] = pTimer;

	return true;
}

bool TimerManager::KillTimer(TimerIdType timerId)
{
	auto it = this->pTimers.find(timerId);
	if (it == this->pTimers.end())
	{
		return false;
	}

	TimerNode* pTimer = it->second;
	if (!pTimer->bRunning)
	{
		return false;
	}

	pTimer->bRunning = false;
	this->pTimers.erase(it);

	return true;
}
