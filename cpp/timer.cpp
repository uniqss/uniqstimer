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

	if (pTimerManager->arrListTimer[wheelIdx][slotIdx] == nullptr)
	{
		pTimerManager->arrListTimer[wheelIdx][slotIdx] = pTimer;
		pTimer->pNext = nullptr;
	}
	else
	{
		pTimer->pNext = pTimerManager->arrListTimer[wheelIdx][slotIdx];
		pTimerManager->arrListTimer[wheelIdx][slotIdx] = pTimer;
	}
}

// 循环该slot里的所有节点，重新AddTimer到管理器中
static void CascadeTimer(TimerManager* pTimerManager, TimerMsType wheelIdx, TimerMsType slotIdx)
{
	TimerNode* pTimer = pTimerManager->arrListTimer[wheelIdx][slotIdx];
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
	pTimerManager->arrListTimer[wheelIdx][slotIdx] = nullptr;
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

		TimerNode* pTimer = arrListTimer[0][idxExecutingSlotIdx];
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
		arrListTimer[0][idxExecutingSlotIdx] = nullptr;

		this->qwCurrentTimeMS++;
	}
}

TimerManager* CreateTimerManager(void)
{
	TimerManager* pTimerManager = new TimerManager();
	if (pTimerManager != NULL)
	{
		for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++)
		{
			for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++)
			{
				pTimerManager->arrListTimer[wheelIdx][slotIdx] = nullptr;
			}
		}

		pTimerManager->qwCurrentTimeMS = UTimerGetCurrentTimeMS();
	}
	return pTimerManager;
}

void DestroyTimerManager(TimerManager* pTimerManager)
{
	if (NULL == pTimerManager)
		return;
	for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++)
	{
		for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++)
		{
			TimerNode* pTimer = pTimerManager->arrListTimer[wheelIdx][slotIdx];
			TimerNode* pNext = nullptr;
			for (;pTimer != nullptr;pTimer = pNext)
			{
				pNext = pTimer->pNext;

				FreeObj(pTimer);
			}
		}
	}
	delete pTimerManager;
}

bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void(*timerFn)(TimerIdType, void*, TimerMsType currTimeMS), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod)
{
	if (NULL == timerFn || NULL == pTimerManager)
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

	auto it = pTimerManager->pTimers.find(timerId);
	if (it != pTimerManager->pTimers.end())
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

	pTimer->qwExpires = pTimerManager->qwCurrentTimeMS + qwDueTime;
	AddTimer(pTimerManager, pTimer, 0, 0, EAddTimerSource_Create_NewAlloc);
	pTimerManager->pTimers[timerId] = pTimer;

	return true;
}

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId)
{
	if (pTimerManager == nullptr)
	{
		return false;
	}
	auto it = pTimerManager->pTimers.find(timerId);
	if (it == pTimerManager->pTimers.end())
	{
		return false;
	}

	TimerNode* pTimer = it->second;
	if (!pTimer->bRunning)
	{
		return false;
	}

	pTimer->bRunning = false;
	pTimerManager->pTimers.erase(it);

	return true;
}
