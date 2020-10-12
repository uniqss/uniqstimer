#include "Timer.h"

#include <stddef.h>
#include <stdlib.h>
#include<chrono>

int64_t UTimerGetCurrentTimeMS(void)
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

static void DeleteArrayListTimer(ListTimer* arrListTimer, TimerMsType uSize)
{
	ListTimer listTmr, * pListTimer;
	TimerNode* pTmr;
	TimerMsType idx;

	for (idx = 0; idx < uSize; idx++)
	{
		ListTimerReplaceInit(&arrListTimer[idx], &listTmr);
		pListTimer = listTmr.pNext;
		while (pListTimer != &listTmr)
		{
			pTmr = (TimerNode*)((uint8_t*)pListTimer - offsetof(TimerNode, ltTimer));
			pListTimer = pListTimer->pNext;
			free(pTmr);
		}
	}
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
	TimerMsType idx, uJiffies;
	ListTimer listTmrExpire, * pListTmrExpire;
	TimerNode* pTmr;

	if (NULL == lpTimerManager)
		return;
	uJiffies = UTimerGetCurrentTimeMS();
	lpTimerManager->lock.lock();
	while (TIME_AFTER_EQ(uJiffies, lpTimerManager->currentTimeMS))
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
			else free(pTmr);
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

TimerManager* CreateTimerManager(void)
{
	TimerManager* lpTimerMgr = new TimerManager();
	if (lpTimerMgr != NULL)
	{
		lpTimerMgr->threadWorking = true;
		lpTimerMgr->currentTimeMS = UTimerGetCurrentTimeMS();
		InitArrayListTimer(lpTimerMgr->arrListTimer1, sizeof(lpTimerMgr->arrListTimer1) / sizeof(lpTimerMgr->arrListTimer1[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer2, sizeof(lpTimerMgr->arrListTimer2) / sizeof(lpTimerMgr->arrListTimer2[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer3, sizeof(lpTimerMgr->arrListTimer3) / sizeof(lpTimerMgr->arrListTimer3[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer4, sizeof(lpTimerMgr->arrListTimer4) / sizeof(lpTimerMgr->arrListTimer4[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer5, sizeof(lpTimerMgr->arrListTimer5) / sizeof(lpTimerMgr->arrListTimer5[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer6, sizeof(lpTimerMgr->arrListTimer6) / sizeof(lpTimerMgr->arrListTimer6[0]));
		lpTimerMgr->threadWaitingTerminateOK = false;
		lpTimerMgr->thread = std::move(new std::thread(ThreadRunTimer, lpTimerMgr));
	}
	return lpTimerMgr;
}

void DestroyTimerManager(TimerManager* lpTimerManager)
{
	if (NULL == lpTimerManager)
		return;
	lpTimerManager->threadWorking = false;
	while (!lpTimerManager->threadWaitingTerminateOK)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	DeleteArrayListTimer(lpTimerManager->arrListTimer1, sizeof(lpTimerManager->arrListTimer1) / sizeof(lpTimerManager->arrListTimer1[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer2, sizeof(lpTimerManager->arrListTimer2) / sizeof(lpTimerManager->arrListTimer2[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer3, sizeof(lpTimerManager->arrListTimer3) / sizeof(lpTimerManager->arrListTimer3[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer4, sizeof(lpTimerManager->arrListTimer4) / sizeof(lpTimerManager->arrListTimer4[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer5, sizeof(lpTimerManager->arrListTimer5) / sizeof(lpTimerManager->arrListTimer5[0]));
	DeleteArrayListTimer(lpTimerManager->arrListTimer6, sizeof(lpTimerManager->arrListTimer6) / sizeof(lpTimerManager->arrListTimer6[0]));
	free(lpTimerManager);
}

// 创建一个定时器  uDueTime 首次触发的超时时间间隔  uPeriod 定时器循环周期，若为0，则该定时器只运行一次
TimerNode* CreateTimer(TimerManager* lpTimerManager, void (*timerFn)(void*), void* pParam, TimerMsType uDueTime, TimerMsType uPeriod)
{
	TimerNode* pTmr = NULL;
	if (NULL == timerFn || NULL == lpTimerManager)
		return NULL;
	pTmr = new TimerNode();
	if (pTmr != NULL)
	{
		pTmr->uPeriod = uPeriod;
		pTmr->timerFn = timerFn;
		pTmr->pParam = pParam;


		lpTimerManager->lock.lock();
		pTmr->uExpires = lpTimerManager->currentTimeMS + uDueTime;
		AddTimer(lpTimerManager, pTmr);
		lpTimerManager->lock.unlock();
	}
	return pTmr;
}

//删除定时器
int32_t DeleteTimer(TimerManager* lpTimerManager, TimerNode* lpTimer)
{
	ListTimer* pListTmr;
	if (NULL != lpTimerManager && NULL != lpTimer)
	{
		lpTimerManager->lock.lock();
		pListTmr = &lpTimer->ltTimer;
		pListTmr->pPrev->pNext = pListTmr->pNext;
		pListTmr->pNext->pPrev = pListTmr->pPrev;
		free(lpTimer);
		lpTimerManager->lock.unlock();
		return 0;
	}
	else
	{
		return -1;
	}
}
