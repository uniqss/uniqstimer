#include "Timer.h"

#include "timer_mem.h"

#include <stddef.h>
#include <stdlib.h>
#include<chrono>
#include <cassert>


#ifdef UNIQS_DEBUG_TIMER
TimerMsType DebugDiffTimeMs = 0;
#endif

TimerMsType UTimerGetCurrentTimeMS(void)
{
	auto time_now = std::chrono::system_clock::now();
	auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
#ifdef UNIQS_DEBUG_TIMER
	return (TimerMsType)duration_in_ms.count() - DebugDiffTimeMs;
#else
	return (TimerMsType)duration_in_ms.count();
#endif
}

void OnTimerError(const std::string& err)
{
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "OnTimer error err:" << err;
#endif

	throw std::logic_error("OnTimerError" + err);

	//printf("OnTimer error err:%s\n", err.c_str());
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
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "AddTimer timerId:"<<pTimer->qwTimerId<<"fromWheelIdx:" << fromWheelIdx << " fromSlotIdx:" << fromSlotIdx << " source:" << pszAddTimerSource[source];
#endif

	TimerMsType wheelIdx = 0;
	TimerMsType slotIdx = 0;

	TimerMsType qwExpires = pTimer->qwExpires;
	TimerMsType qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS;

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
	bool negative = false;
#endif

	if (false) {}
	else if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (0 + 1)))
	{
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 0)) & TIMER_MASK;
		wheelIdx = 0;
	}
	else if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (1 + 1)))
	{
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 1)) & TIMER_MASK;
		wheelIdx = 1;
	}
	else if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (2 + 1)))
	{
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 2)) & TIMER_MASK;
		wheelIdx = 2;
	}
	else if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (3 + 1)))
	{
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 3)) & TIMER_MASK;
		wheelIdx = 3;
	}
	else if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (4 + 1)))
	{
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 4)) & TIMER_MASK;
		wheelIdx = 4;
	}
	else if (qwDueTime < 0)
	{
		wheelIdx = 0;
		slotIdx = pTimerManager->qwCurrentTimeMS & TIMER_MASK;
#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		negative = true;
#endif
	}
	else
	{
		OnTimerError("AddTimer this should not happen");
		return;
	}

#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "AddTimer timerId:" << pTimer->qwTimerId << " source:" << pszAddTimerSource[source] << " fromWheelIdx:" << fromWheelIdx << " fromSlotIdx:" << fromSlotIdx <<
		" wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx << " negative:" << negative << " qwDueTime:" << qwDueTime <<
		" qwExpires:" << qwExpires << " qwCurrentTimeMS:" << pTimerManager->qwCurrentTimeMS;
#endif

#if defined (UNIQS_DEBUG_TIMER )
	printf("AddTimer source:%s fromWheelIdx:%llu fromSlotIdx:%llu wheelIdx:%llu slotIdx:%llu negative:%d qwDueTime:%llu qwExpires:%llu qwCurrentTimeMS:%llu\n",
		pszAddTimerSource[source], fromWheelIdx, fromSlotIdx, wheelIdx, slotIdx, int(negative), qwDueTime, qwExpires, pTimerManager->qwCurrentTimeMS);
#endif
	//printf("AddTimer qwExpires:%llu qwDueTime:%llu i:%llu pTimerManager->qwCurrentTimeMS:%llu\n", qwExpires, qwDueTime, i, pTimerManager->qwCurrentTimeMS);
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
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "CascadeTimer begin wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif
#ifdef UNIQS_DEBUG_TIMER
	printf("CascadeTimer begin wheelIdx:%llu slotIdx:%llu \n", wheelIdx, slotIdx);
#endif
	TimerNode* pTimer = pTimerManager->arrListTimer[wheelIdx][slotIdx];
	TimerNode* pNext = nullptr;
	for (;pTimer != nullptr;pTimer = pNext)
	{
		pNext = pTimer->pNext;

		if (pTimer->bRunning)
		{
#ifdef UNIQS_LOG_EVERYTHING
			LOG(INFO) << "CascadeTimer running AddTimer qwTimerId:" << pTimer->qwTimerId << " wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif
			AddTimer(pTimerManager, pTimer, wheelIdx, slotIdx, EAddTimerSource_Cascade);
		}
		else
		{
#ifdef UNIQS_LOG_EVERYTHING
			LOG(INFO) << "CascadeTimer killed delete qwTimerId:" << pTimer->qwTimerId << " wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif
			// timer还没被放到最终执行轮就已经被kill掉了，需要释放掉
			//pTimerManager->pTimers.erase(pTimer->qwTimerId);
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
				pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam);
				if (pTimer->qwPeriod != 0)
				{
					pTimer->qwExpires = this->qwCurrentTimeMS + pTimer->qwPeriod;
					AddTimer(this, pTimer, 0, idxExecutingSlotIdx, EAddTimerSource_TimeoutReadd);
				}
				else
				{
#ifdef UNIQS_LOG_EVERYTHING
					LOG(INFO) << "TimerManager::Run running state. last trigger. kill delete timerId:" << timerId << " pTimer->qwPeriod:" <<
						pTimer->qwPeriod << " pTimer->qwExpires:" << pTimer->qwExpires << " currTimeMS:" << currTimeMS;
#endif
					FreeObj(pTimer);
					pTimers.erase(timerId);
				}
			}
			else
			{
#ifdef UNIQS_LOG_EVERYTHING
				LOG(INFO) << "TimerManager::Run killed state. delete timerId:" << timerId << " pTimer->qwPeriod:" <<
					pTimer->qwPeriod << " pTimer->qwExpires:" << pTimer->qwExpires << " currTimeMS:" << currTimeMS;
#endif
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

bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void(*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod)
{
	if (NULL == timerFn || NULL == pTimerManager)
		return false;

#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "CreateTimer timerId:" << timerId << " qwDueTime:" <<
		qwDueTime << " qwPeriod:" << qwPeriod << " pTimerManager->qwCurrentTimeMS:" << pTimerManager->qwCurrentTimeMS;
#endif
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
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "KillTimer timerId:" << timerId << " exists:" <<
		(it == pTimerManager->pTimers.end());
#endif
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
