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

	decltype(&pTimerManager->arrListTimer[0][0]) pSlot;
	TimerMsType i = 0, qwDueTime, qwExpires;

	qwExpires = pTimer->qwExpires;
	qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS;

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
	TimerMsType wheelIdx = 0;
	TimerMsType slotIdx = 0;
	bool negative = false;
#endif

	if (false) {}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (0 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 0)) & TIMER_MASK;
		pSlot = &pTimerManager->arrListTimer[0][i];

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		wheelIdx = 0;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (1 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 1)) & TIMER_MASK;
		pSlot = &pTimerManager->arrListTimer[1][i];

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		wheelIdx = 1;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (2 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 2)) & TIMER_MASK;
		pSlot = &pTimerManager->arrListTimer[2][i];

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		wheelIdx = 2;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (3 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 3)) & TIMER_MASK;
		pSlot = &pTimerManager->arrListTimer[3][i];

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		wheelIdx = 3;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (4 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 4)) & TIMER_MASK;
		pSlot = &pTimerManager->arrListTimer[4][i];

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		wheelIdx = 4;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < 0)
	{
		pSlot = &pTimerManager->arrListTimer[0][(pTimerManager->qwCurrentTimeMS & TIMER_MASK)];

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		wheelIdx = 0;
		slotIdx = i;
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
	pSlot->push_back(pTimer);
}

// 循环该slot里的所有节点，重新AddTimer到管理器中
static void CascadeTimer(TimerManager* pTimerManager, std::list<TimerNode*>& rSlotTimers, TimerMsType wheelIdx, TimerMsType slotIdx)
{
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "CascadeTimer begin wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif
#ifdef UNIQS_DEBUG_TIMER
	printf("CascadeTimer begin wheelIdx:%llu slotIdx:%llu \n", wheelIdx, slotIdx);
#endif
	TimerNode* pTimer = nullptr;
	for (auto it = rSlotTimers.begin(); it != rSlotTimers.end();++it)
	{
		pTimer = *it;
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
	rSlotTimers.clear();
}

void TimerManager::Run()
{
	TimerMsType idx = 0, idxTmp = 0, currTimeMS;

	currTimeMS = UTimerGetCurrentTimeMS();
	TimerIdType timerId = 0;
	while (currTimeMS >= qwCurrentTimeMS)
	{
		idx = qwCurrentTimeMS & TIMER_MASK;

		idxTmp = idx;
		for (TimerMsType i = 0; i < 4 && idxTmp == 0; i++)
		{
			idxTmp = (qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
			CascadeTimer(this, arrListTimer[i + 1][idxTmp], i + 1, idxTmp);
		}

		auto& rSlotTimers = arrListTimer[0][idx];
		for (auto it = rSlotTimers.begin();it != rSlotTimers.end();)
		{
			TimerNode* pTimer = *it;
			timerId = pTimer->qwTimerId;
			if (pTimer->bRunning)
			{
				pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam);
				if (pTimer->qwPeriod != 0)
				{
					pTimer->qwExpires = qwCurrentTimeMS + pTimer->qwPeriod;
					AddTimer(this, pTimer, 0, idx, EAddTimerSource_TimeoutReadd);
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

			it = rSlotTimers.erase(it);
		}

		qwCurrentTimeMS++;
	}
}

TimerManager* CreateTimerManager(void)
{
	TimerManager* pTimerMgr = new TimerManager();
	if (pTimerMgr != NULL)
	{
		pTimerMgr->qwCurrentTimeMS = UTimerGetCurrentTimeMS();
		for (auto i = 0; i < TIMER_WHEEL_COUNT; i++)
		{
		}
	}
	return pTimerMgr;
}

void DestroyTimerManager(TimerManager* pTimerManager)
{
	if (NULL == pTimerManager)
		return;
	for (auto i = 0; i < TIMER_WHEEL_COUNT; i++)
	{
		for (auto j = 0; j < TIMER_SLOT_COUNT_PER_WHEEL; j++)
		{
			auto& rSlot = pTimerManager->arrListTimer[i][j];
			for (auto it = rSlot.begin();it != rSlot.end();++it)
			{
				FreeObj(*it);
			}
			rSlot.clear();
		}
	}
	delete pTimerManager;
}

bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void(*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod)
{
	TimerNode* pTimer = NULL;
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

	pTimer = AllocObj();
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
	TimerNode* pTimer;
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

	pTimer = it->second;
	if (!pTimer->bRunning)
	{
		return false;
	}

	pTimer->bRunning = false;
	pTimerManager->pTimers.erase(it);

	return true;
}
