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

static void ListTimerInsert(TimerNode* pNew, TimerNode* pPrev, TimerNode* pNext)
{
	pNext->pPrev = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = pPrev;
	pPrev->pNext = pNew;
}

static void ListTimerInsertTail(TimerNode* pNew, TimerNode* pHead)
{
	ListTimerInsert(pNew, pHead->pPrev, pHead);
}

static void ListTimerReplace(TimerNode* pOld, TimerNode* pNew)
{
	pNew->pNext = pOld->pNext;
	pNew->pNext->pPrev = pNew;
	pNew->pPrev = pOld->pPrev;
	pNew->pPrev->pNext = pNew;
}

static void ListTimerReplaceInit(TimerNode* pOld, TimerNode* pNew)
{
	ListTimerReplace(pOld, pNew);
	pOld->pNext = pOld;
	pOld->pPrev = pOld;
}

static void InitArrayListTimer(TimerNode* arrListTimer, TimerMsType nSize)
{
	TimerMsType i;
	for (i = 0; i < nSize; i++)
	{
		arrListTimer[i].pPrev = &arrListTimer[i];
		arrListTimer[i].pNext = &arrListTimer[i];
	}
}

static void SetTimerStateAnd(TimerNode* pTimer, int state)
{
#ifdef UNIQS_DEBUG_TIMER
	auto oldState = pTimer->state;
#endif
	pTimer->state &= state;
#ifdef UNIQS_DEBUG_TIMER
	printf("SetTimerStateAnd timerId:%llu oldState:%d newState:%d state:%d\n", pTimer->qwTimerId, oldState, pTimer->state, state);
#endif
}

static void SetTimerStateOr(TimerNode* pTimer, int state)
{
#ifdef UNIQS_DEBUG_TIMER
	auto oldState = pTimer->state;
#endif
	pTimer->state |= state;
#ifdef UNIQS_DEBUG_TIMER
	printf("SetTimerStateOr oldState:%d newState:%d state:%d\n", oldState, pTimer->state, state);
#endif
}

static void DeleteArrayListTimer(TimerNode* arrListTimer, TimerMsType uSize)
{
	TimerNode listTmr, * pListTimer;
	TimerNode* pTmr;
	TimerMsType idx;

	for (idx = 0; idx < uSize; idx++)
	{
		ListTimerReplaceInit(&arrListTimer[idx], &listTmr);
		pListTimer = listTmr.pNext;
		while (pListTimer != &listTmr)
		{
			pTmr = pListTimer;
			pListTimer = pListTimer->pNext;
			FreeObj(pTmr);
		}
	}
}

enum EAddTimerSource
{
	EAddTimerSource_Create_Reuse,
	EAddTimerSource_Create_NewAlloc,
	EAddTimerSource_Cascade,
	EAddTimerSource_TimeoutReadd,
};
const char* pszAddTimerSource[] = {
	"EAddTimerSource_Create_Reuse",
	"EAddTimerSource_Create_NewAlloc",
	"EAddTimerSource_Cascade",
	"EAddTimerSource_TimeoutReadd",
};

static void AddTimer(TimerManager* pTimerManager, TimerNode* pTimer, TimerMsType fromWheelIdx, TimerMsType fromSlotIdx, EAddTimerSource source)
{
	TimerNode* pHead;
	TimerMsType i = 0, qwDueTime, qwExpires;

	qwExpires = pTimer->qwExpires;
	qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS;

#ifdef UNIQS_DEBUG_TIMER
	TimerMsType wheelIdx = 0;
	TimerMsType slotIdx = 0;
	bool negative = false;
#endif

	if (false) {}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (0 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 0)) & TIMER_MASK;
		pHead = &pTimerManager->arrListTimer[0][i];

#ifdef UNIQS_DEBUG_TIMER
		wheelIdx = 0;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (1 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 1)) & TIMER_MASK;
		pHead = &pTimerManager->arrListTimer[1][i];

#ifdef UNIQS_DEBUG_TIMER
		wheelIdx = 1;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (2 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 2)) & TIMER_MASK;
		pHead = &pTimerManager->arrListTimer[2][i];

#ifdef UNIQS_DEBUG_TIMER
		wheelIdx = 2;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (3 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 3)) & TIMER_MASK;
		pHead = &pTimerManager->arrListTimer[3][i];

#ifdef UNIQS_DEBUG_TIMER
		wheelIdx = 3;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (4 + 1)))
	{
		i = (qwExpires >> (TIMER_BITS_PER_WHEEL * 4)) & TIMER_MASK;
		pHead = &pTimerManager->arrListTimer[4][i];

#ifdef UNIQS_DEBUG_TIMER
		wheelIdx = 4;
		slotIdx = i;
#endif
	}
	else if (qwDueTime < 0)
	{
		pHead = &pTimerManager->arrListTimer[0][(pTimerManager->qwCurrentTimeMS & TIMER_MASK)];

#ifdef UNIQS_DEBUG_TIMER
		wheelIdx = 0;
		slotIdx = i;
		negative = true;
#endif
	}
	else
	{
		throw std::logic_error("AddTimer this should not happen \n");
		return;
	}

#ifdef UNIQS_DEBUG_TIMER
	printf("AddTimer source:%s fromWheelIdx:%llu fromSlotIdx:%llu wheelIdx:%llu slotIdx:%llu negative:%d qwDueTime:%llu qwExpires:%llu qwCurrentTimeMS:%llu\n",
		pszAddTimerSource[source], fromWheelIdx, fromSlotIdx, wheelIdx, slotIdx, int(negative), qwDueTime, qwExpires, pTimerManager->qwCurrentTimeMS);
#endif
	//printf("AddTimer qwExpires:%llu qwDueTime:%llu i:%llu pTimerManager->qwCurrentTimeMS:%llu\n", qwExpires, qwDueTime, i, pTimerManager->qwCurrentTimeMS);
	ListTimerInsertTail(pTimer, pHead);
}

// 循环该slot里的所有节点，重新AddTimer到管理器中
static void CascadeTimer(TimerManager* pTimerManager, TimerNode ppTimers[][TIMER_COUNT_PER_WHEEL], TimerMsType wheelIdx, TimerMsType slotIdx)
{
#ifdef UNIQS_DEBUG_TIMER
	printf("CascadeTimer begin wheelIdx:%llu slotIdx:%llu \n", wheelIdx, slotIdx);
#endif
	TimerNode& rSlotTimerHead = ppTimers[wheelIdx][slotIdx];

	//printf("CascadeTimer idx:%llu\n", idx);
	TimerNode oTimerHead, * pTimerNext;
	TimerNode* pTimer;

#if 0
	ListTimerReplaceInit(&rSlotTimerHead, &oTimerHead);
#else
	oTimerHead.pNext = rSlotTimerHead.pNext;
	oTimerHead.pNext->pPrev = &oTimerHead;
	oTimerHead.pPrev = rSlotTimerHead.pPrev;
	oTimerHead.pPrev->pNext = &oTimerHead;
	rSlotTimerHead.pNext = &rSlotTimerHead;
	rSlotTimerHead.pPrev = &rSlotTimerHead;
#endif

#ifdef UNIQS_DEBUG_TIMER
	int slotTimerListIdx = 0;
#endif

	pTimerNext = oTimerHead.pNext;
	while (pTimerNext != &oTimerHead)
	{
		pTimer = pTimerNext;
		pTimerNext = pTimerNext->pNext;

#ifdef UNIQS_DEBUG_TIMER
		slotTimerListIdx++;
		printf("CascadeTimer looping wheelIdx:%llu slotIdx:%llu slotTimerListIdx:%d qwTimerId:%llu pTimer:%llu\n"
			, wheelIdx, slotIdx, slotTimerListIdx, pTimer->qwTimerId, (int64_t)pTimer);
#endif

		if (pTimer->state & ETimerState_Running)
		{
			AddTimer(pTimerManager, pTimer, wheelIdx, slotIdx, EAddTimerSource_Cascade);
		}
		else
		{
			throw std::logic_error("CascadeTimer this should not happen.\n");
		}
	}
#ifdef UNIQS_DEBUG_TIMER
	printf("CascadeTimer end wheelIdx:%llu slotIdx:%llu, count:%d \n", wheelIdx, slotIdx, slotTimerListIdx);
#endif
}

void TimerManager::Run()
{
	TimerMsType idx = 0, idxTmp = 0, currTimeMS;
	TimerNode oTimerHead, * pTimerNext;
	TimerNode* pTimer;

#define INDEX(N) ((qwCurrentTimeMS >> ((N + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK)

	currTimeMS = UTimerGetCurrentTimeMS();
	while (currTimeMS >= qwCurrentTimeMS)
	{
		idx = qwCurrentTimeMS & TIMER_MASK;

		idxTmp = idx;
		for (TimerMsType i = 0; i < 4 && idxTmp == 0; i++)
		{
			idxTmp = INDEX(i);
			CascadeTimer(this, arrListTimer, i + 1, idxTmp);
		}

		pTimerNext = &oTimerHead;
		ListTimerReplaceInit(&arrListTimer[0][idx], pTimerNext);
		pTimerNext = pTimerNext->pNext;
#ifdef UNIQS_DEBUG_TIMER
		int slotTimerListIdx = 0;
#endif
		while (pTimerNext != &oTimerHead)
		{
			pTimer = pTimerNext;
			pTimerNext = pTimerNext->pNext;
#ifdef UNIQS_DEBUG_TIMER
			slotTimerListIdx++;
			printf("TimerManager::Run pre timerFn timerId:%llu slotTimerListIdx:%d state:%d\n", pTimer->qwTimerId, slotTimerListIdx, pTimer->state);
#endif
			pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam);
#ifdef UNIQS_DEBUG_TIMER
			printf("TimerManager::Run post timerFn timerId:%llu slotTimerListIdx:%d state:%d\n", pTimer->qwTimerId, slotTimerListIdx, pTimer->state);
#endif
			if ((pTimer->state & ETimerState_FrameChanged) != 0)
			{
				SetTimerStateAnd(pTimer, ~ETimerState_FrameChanged);
				continue;
			}

			if (pTimer->qwPeriod != 0)
			{
				pTimer->qwExpires = qwCurrentTimeMS + pTimer->qwPeriod;
				AddTimer(this, pTimer, 0, idx, EAddTimerSource_TimeoutReadd);
			}
			else
			{
				pTimer->pPrev->pNext = pTimer->pNext;
				pTimer->pNext->pPrev = pTimer->pPrev;

				pTimer->pPrev = nullptr;
				pTimer->pNext = nullptr;

				SetTimerStateAnd(pTimer, ~ETimerState_Running);
				SetTimerStateOr(pTimer, ETimerState_Killed);
				pPendingDeleteTimers.insert(pTimer->qwTimerId);
			}
		}
		pendingFree();
		qwCurrentTimeMS++;
	}
}

void TimerManager::pendingFree()
{
	for (auto timerId : pPendingDeleteTimers)
	{
#ifdef UNIQS_DEBUG_TIMER
		printf("TimerManager::pendingFree timerId:%llu\n", timerId);
#endif

		auto it = pTimers.find(timerId);
		if (it != pTimers.end())
		{
			FreeObj(it->second);
			pTimers.erase(it);
		}
		else
		{
			throw std::logic_error("TimerManager::pendingFree this should not happen. ");
		}
	}
	pPendingDeleteTimers.clear();
}

TimerManager* CreateTimerManager(void)
{
	TimerManager* pTimerMgr = new TimerManager();
	if (pTimerMgr != NULL)
	{
		pTimerMgr->qwCurrentTimeMS = UTimerGetCurrentTimeMS();
		for (auto i = 0; i < TIMER_WHEEL_COUNT; i++)
		{
			InitArrayListTimer(pTimerMgr->arrListTimer[i], TIMER_COUNT_PER_WHEEL);
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
		DeleteArrayListTimer(pTimerManager->arrListTimer[i], TIMER_BITS_PER_WHEEL);
	}
	delete pTimerManager;
}

bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void(*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod)
{
	TimerNode* pTimer = NULL;
	if (NULL == timerFn || NULL == pTimerManager)
		return false;

	// 两者都为0,无意义
	if (qwDueTime == 0 && qwPeriod == 0)
	{
		return false;
	}

	// 如果创建重复性定时器，又设置触发时间为0,就不要立即触发，而是等到第一个周期后再触发。立即触发的逻辑可以立即写。
	if (qwDueTime == 0)
	{
		qwDueTime = qwPeriod;
	}

	auto it = pTimerManager->pTimers.find(timerId);
	bool bExists = false;
	if (it != pTimerManager->pTimers.end())
	{
		if (it->second->state & ETimerState_Running)
		{
			return false;
		}

		bExists = true;
		pTimer = it->second;
	}

	if (!bExists)
	{
		pTimer = AllocObj();
	}
	else
	{
		pTimerManager->pPendingDeleteTimers.erase(timerId);
	}

	if (pTimer != NULL)
	{
		pTimer->qwPeriod = qwPeriod;
		pTimer->timerFn = timerFn;
		pTimer->pParam = pParam;
		pTimer->qwTimerId = timerId;
		
		SetTimerStateAnd(pTimer, ~ETimerState_Killed);
		SetTimerStateOr(pTimer, ETimerState_Running);

		pTimer->qwExpires = pTimerManager->qwCurrentTimeMS + qwDueTime;
		AddTimer(pTimerManager, pTimer, 0, 0, bExists ? EAddTimerSource_Create_Reuse : EAddTimerSource_Create_NewAlloc);
		pTimerManager->pTimers[timerId] = pTimer;
	}
	else
	{
		throw std::logic_error("CreateTimer AllocObj failed. \n");
		return false;
	}
	return true;
}

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId)
{
	TimerNode* pTimer;
	auto it = pTimerManager->pTimers.find(timerId);
	if (NULL != pTimerManager && it != pTimerManager->pTimers.end() && it->second->state & ETimerState_Running)
	{
		pTimer = it->second;

		pTimer->pPrev->pNext = pTimer->pNext;
		pTimer->pNext->pPrev = pTimer->pPrev;

		pTimer->pPrev = nullptr;
		pTimer->pNext = nullptr;

		SetTimerStateAnd(pTimer, ~ETimerState_Running);
		SetTimerStateOr(pTimer, ETimerState_Killed | ETimerState_FrameChanged);
		pTimerManager->pPendingDeleteTimers.insert(timerId);

		return true;
	}
	else
	{
		return false;
	}
}
