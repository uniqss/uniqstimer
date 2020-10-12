#include "Timer.h"

#include <stddef.h>
#include <stdlib.h>
#include<chrono>

#include "impl/timernode.h"

uint64_t UTimerGetCurrentTimeMS(void)
{
	auto time_now = std::chrono::system_clock::now();
	auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
	return duration_in_ms.count();
}

static void ListTimerInsert(ListTimer* pNew, ListTimer* pPrev, ListTimer* pNext)
{
	pNext->pPrev = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = pPrev;
	pPrev->pNext = pNew;
}

static void ListTimerInsertHead(ListTimer* pNew, ListTimer* pHead)
{
	ListTimerInsert(pNew, pHead, pHead->pNext);
}

static void ListTimerInsertTail(ListTimer* pNew, ListTimer* pHead)
{
	ListTimerInsert(pNew, pHead->pPrev, pHead);
}

static void ListTimerReplace(ListTimer* pOld, ListTimer* pNew)
{
	pNew->pNext = pOld->pNext;
	pNew->pNext->pPrev = pNew;
	pNew->pPrev = pOld->pPrev;
	pNew->pPrev->pNext = pNew;
}

static void ListTimerReplaceInit(ListTimer* pOld, ListTimer* pNew)
{
	ListTimerReplace(pOld, pNew);
	pOld->pNext = pOld;
	pOld->pPrev = pOld;
}

static void InitArrayListTimer(ListTimer* arrListTimer, TimerMsType nSize)
{
	TimerMsType i;
	for (i = 0; i < nSize; i++)
	{
		arrListTimer[i].pPrev = &arrListTimer[i];
		arrListTimer[i].pNext = &arrListTimer[i];
	}
}

// 内存统一由池管理
static void DeleteArrayListTimer(ListTimer* arrListTimer, TimerMsType uSize)
{
	//ListTimer listTmr, * pListTimer;
	//TimerNode* pTmr;
	//TimerMsType idx;

	//for (idx = 0; idx < uSize; idx++)
	//{
	//	ListTimerReplaceInit(&arrListTimer[idx], &listTmr);
	//	pListTimer = listTmr.pNext;
	//	while (pListTimer != &listTmr)
	//	{
	//		pTmr = (TimerNode*)((uint8_t*)pListTimer - offsetof(TimerNode, ltTimer));
	//		pListTimer = pListTimer->pNext;
	//		delete pTmr;
	//	}
	//}
}

static void AddTimer(TimerManager* lpTimerManager, TimerNode* pTmr)
{
	ListTimer* pHead;
	TimerMsType i, uDueTime, uExpires;

	uExpires = pTmr->uExpires;
	uDueTime = uExpires - lpTimerManager->currentTimeMS;
	if (uDueTime < TVR_SIZE)
	{
		i = uExpires & TVR_MASK;
		pHead = &lpTimerManager->arrListTimer1[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 1 * TVN_BITS))
	{
		i = (uExpires >> TVR_BITS) & TVN_MASK;
		pHead = &lpTimerManager->arrListTimer2[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 2 * TVN_BITS))
	{
		i = (uExpires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		pHead = &lpTimerManager->arrListTimer3[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 3 * TVN_BITS))
	{
		i = (uExpires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		pHead = &lpTimerManager->arrListTimer4[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 4 * TVN_BITS))
	{
		i = (uExpires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		pHead = &lpTimerManager->arrListTimer5[i];
	}
	else if ((signed long)uDueTime < 0)
	{
		pHead = &lpTimerManager->arrListTimer1[(lpTimerManager->currentTimeMS & TVR_MASK)];
	}
	else
	{
		if (uDueTime > TimerMsType(0xffffffffffffffffULL))
		{
			uDueTime = TimerMsType(0xffffffffffffffffULL);
			uExpires = uDueTime + lpTimerManager->currentTimeMS;
		}
		i = (uExpires >> (TVR_BITS + 4 * TVN_BITS)) & TVN_MASK;
		pHead = &lpTimerManager->arrListTimer5[i];
	}
	ListTimerInsertTail(&pTmr->ltTimer, pHead);
}

static TimerMsType CascadeTimer(TimerManager* lpTimerManager, ListTimer* arrListTimer, TimerMsType idx)
{
	ListTimer listTmr, * pListTimer;
	TimerNode* pTmr;

	ListTimerReplaceInit(&arrListTimer[idx], &listTmr);
	pListTimer = listTmr.pNext;
	while (pListTimer != &listTmr)
	{
		pTmr = (TimerNode*)((uint8_t*)pListTimer - offsetof(TimerNode, ltTimer));
		pListTimer = pListTimer->pNext;
		AddTimer(lpTimerManager, pTmr);
	}
	return idx;
}

static void RunTimer(TimerManager* lpTimerManager)
{
#define INDEX(N) ((lpTimerManager->currentTimeMS >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)
	TimerMsType idx, timeMS;
	ListTimer listTmrExpire, * pListTmrExpire;
	TimerNode* pTmr;

	if (NULL == lpTimerManager)
		return;
	timeMS = UTimerGetCurrentTimeMS();
	lpTimerManager->lock.lock();
	while (TIME_AFTER_EQ(timeMS, lpTimerManager->currentTimeMS))
	{
		idx = lpTimerManager->currentTimeMS & TVR_MASK;
		if (!idx &&
			!CascadeTimer(lpTimerManager, lpTimerManager->arrListTimer2, INDEX(0)) &&
			!CascadeTimer(lpTimerManager, lpTimerManager->arrListTimer3, INDEX(1)) &&
			!CascadeTimer(lpTimerManager, lpTimerManager->arrListTimer4, INDEX(2)) &&
			!CascadeTimer(lpTimerManager, lpTimerManager->arrListTimer5, INDEX(3)) &&
			true)
		{
			CascadeTimer(lpTimerManager, lpTimerManager->arrListTimer6, INDEX(4));
		}
		pListTmrExpire = &listTmrExpire;
		ListTimerReplaceInit(&lpTimerManager->arrListTimer1[idx], pListTmrExpire);
		pListTmrExpire = pListTmrExpire->pNext;
		while (pListTmrExpire != &listTmrExpire)
		{
			pTmr = (TimerNode*)((uint8_t*)pListTmrExpire - offsetof(TimerNode, ltTimer));
			pListTmrExpire = pListTmrExpire->pNext;
			pTmr->timerFn(pTmr->pParam);
			if (pTmr->uPeriod != 0)
			{
				pTmr->uExpires = lpTimerManager->currentTimeMS + pTmr->uPeriod;
				AddTimer(lpTimerManager, pTmr);
			}
			else
			{
				delete pTmr;
			}
		}
		lpTimerManager->currentTimeMS++;
	}
	lpTimerManager->lock.unlock();
}

static void* ThreadRunTimer(void* pParam)
{
	TimerManager* pTimerMgr;

	pTimerMgr = (TimerManager*)pParam;
	if (pTimerMgr == NULL)
		return NULL;
	while (pTimerMgr->threadWorking)
	{
		RunTimer(pTimerMgr);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	pTimerMgr->threadWaitingTerminateOK = true;
	return NULL;
}

TimerManager* CreateTimerManager(bool internalThraed)
{
	TimerManager* lpTimerMgr = new TimerManager();
	if (lpTimerMgr != NULL)
	{
		if (internalThraed)
		{
			lpTimerMgr->internalThread = internalThraed;
			lpTimerMgr->threadWorking = true;
			lpTimerMgr->currentTimeMS = UTimerGetCurrentTimeMS();
		}
		InitArrayListTimer(lpTimerMgr->arrListTimer1, sizeof(lpTimerMgr->arrListTimer1) / sizeof(lpTimerMgr->arrListTimer1[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer2, sizeof(lpTimerMgr->arrListTimer2) / sizeof(lpTimerMgr->arrListTimer2[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer3, sizeof(lpTimerMgr->arrListTimer3) / sizeof(lpTimerMgr->arrListTimer3[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer4, sizeof(lpTimerMgr->arrListTimer4) / sizeof(lpTimerMgr->arrListTimer4[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer5, sizeof(lpTimerMgr->arrListTimer5) / sizeof(lpTimerMgr->arrListTimer5[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer6, sizeof(lpTimerMgr->arrListTimer6) / sizeof(lpTimerMgr->arrListTimer6[0]));

		if (internalThraed)
		{
			lpTimerMgr->threadWaitingTerminateOK = false;
			lpTimerMgr->thread = std::move(new std::thread(ThreadRunTimer, lpTimerMgr));
		}
	}
	return lpTimerMgr;
}

void DestroyTimerManager(TimerManager* lpTimerManager)
{
	if (NULL == lpTimerManager)
		return;
	if (lpTimerManager->internalThread)
	{
		lpTimerManager->threadWorking = false;
		while (!lpTimerManager->threadWaitingTerminateOK)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	DeleteArrayListTimer(lpTimerManager->arrListTimer1, sizeof(lpTimerManager->arrListTimer1) / sizeof(lpTimerManager->arrListTimer1[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer2, sizeof(lpTimerManager->arrListTimer2) / sizeof(lpTimerManager->arrListTimer2[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer3, sizeof(lpTimerManager->arrListTimer3) / sizeof(lpTimerManager->arrListTimer3[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer4, sizeof(lpTimerManager->arrListTimer4) / sizeof(lpTimerManager->arrListTimer4[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer5, sizeof(lpTimerManager->arrListTimer5) / sizeof(lpTimerManager->arrListTimer5[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer6, sizeof(lpTimerManager->arrListTimer6) / sizeof(lpTimerManager->arrListTimer6[0]));

	lpTimerManager->timerPool.Destroy();

	delete lpTimerManager;
}

bool CreateTimer(TimerIdType timerId, TimerManager* lpTimerManager, void (*timerFn)(void*), void* pParam, TimerMsType uDueTime, TimerMsType uPeriod)
{
	TimerNode* pTimer = NULL;
	if (NULL == timerFn || NULL == lpTimerManager)
		return false;
	if (lpTimerManager->internalThread)
	{
		lpTimerManager->lock.lock();
	}
	pTimer = lpTimerManager->timerPool.CreateObj(timerId);
	if (pTimer != NULL)
	{
		pTimer->uPeriod = uPeriod;
		pTimer->timerFn = timerFn;
		pTimer->pParam = pParam;

		pTimer->uExpires = lpTimerManager->currentTimeMS + uDueTime;
		AddTimer(lpTimerManager, pTimer);
	}
	if (lpTimerManager->internalThread)
	{
		lpTimerManager->lock.unlock();
	}
	return true;
}


bool KillTimer(TimerManager* lpTimerManager, TimerIdType timerId)
{
	ListTimer* pListTmr;
	if (NULL != lpTimerManager && timerId != 0)
	{
		if (lpTimerManager->internalThread)
		{
			lpTimerManager->lock.lock();
		}
		auto lpTimer = lpTimerManager->timerPool.FindObj(timerId);
		if (lpTimer != nullptr)
		{
			pListTmr = &lpTimer->ltTimer;
			pListTmr->pPrev->pNext = pListTmr->pNext;
			pListTmr->pNext->pPrev = pListTmr->pPrev;
			lpTimerManager->timerPool.ReleaseObj(timerId);
		}
		if (lpTimerManager->internalThread)
		{
			lpTimerManager->lock.unlock();
		}
		return true;
	}
	else
	{
		return false;
	}
}
