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

static void ListTimerReplaceInit(ListTimer* pOld, ListTimer* pNew)
{
	pNew->pNext = pOld->pNext;
	pNew->pNext->pPrev = pNew;
	pNew->pPrev = pOld->pPrev;
	pNew->pPrev->pNext = pNew;

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

static void AddTimer(TimerManager* pTimerManager, TimerNode* pTimer)
{
	ListTimer* pHead;
	TimerMsType i, uDueTime, uExpires;

	uExpires = pTimer->expireMS;
	uDueTime = uExpires - pTimerManager->currentTimeMS;
	if (uDueTime < TVR_SIZE)
	{
		i = uExpires & TVR_MASK;
		pHead = &pTimerManager->arrListTimer1[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 1 * TVN_BITS))
	{
		i = (uExpires >> TVR_BITS) & TVN_MASK;
		pHead = &pTimerManager->arrListTimer2[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 2 * TVN_BITS))
	{
		i = (uExpires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		pHead = &pTimerManager->arrListTimer3[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 3 * TVN_BITS))
	{
		i = (uExpires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		pHead = &pTimerManager->arrListTimer4[i];
	}
	else if (uDueTime < TimerMsType(1) << (TVR_BITS + 4 * TVN_BITS))
	{
		i = (uExpires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		pHead = &pTimerManager->arrListTimer5[i];
	}
	else if ((signed long)uDueTime < 0)
	{
		pHead = &pTimerManager->arrListTimer1[(pTimerManager->currentTimeMS & TVR_MASK)];
	}
	else
	{
		if (uDueTime > TimerMsType(0xffffffffffffffffULL))
		{
			uDueTime = TimerMsType(0xffffffffffffffffULL);
			uExpires = uDueTime + pTimerManager->currentTimeMS;
		}
		i = (uExpires >> (TVR_BITS + 4 * TVN_BITS)) & TVN_MASK;
		pHead = &pTimerManager->arrListTimer5[i];
	}
	ListTimerInsert(&pTimer->listTimer, pHead->pPrev, pHead);
}

static TimerMsType CascadeTimer(TimerManager* pTimerManager, ListTimer* arrListTimer, TimerMsType idx)
{
	ListTimer listTmr, * pListTimer;
	TimerNode* pTmr;

	ListTimerReplaceInit(&arrListTimer[idx], &listTmr);
	pListTimer = listTmr.pNext;
	while (pListTimer != &listTmr)
	{
		pTmr = (TimerNode*)((uint8_t*)pListTimer - offsetof(TimerNode, listTimer));
		pListTimer = pListTimer->pNext;
		AddTimer(pTimerManager, pTmr);
	}
	return idx;
}

TimerManager* CreateTimerManager()
{
	TimerManager* lpTimerMgr = new TimerManager();
	if (lpTimerMgr != NULL)
	{
		lpTimerMgr->currentTimeMS = UTimerGetCurrentTimeMS();
		InitArrayListTimer(lpTimerMgr->arrListTimer1, sizeof(lpTimerMgr->arrListTimer1) / sizeof(lpTimerMgr->arrListTimer1[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer2, sizeof(lpTimerMgr->arrListTimer2) / sizeof(lpTimerMgr->arrListTimer2[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer3, sizeof(lpTimerMgr->arrListTimer3) / sizeof(lpTimerMgr->arrListTimer3[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer4, sizeof(lpTimerMgr->arrListTimer4) / sizeof(lpTimerMgr->arrListTimer4[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer5, sizeof(lpTimerMgr->arrListTimer5) / sizeof(lpTimerMgr->arrListTimer5[0]));
		InitArrayListTimer(lpTimerMgr->arrListTimer6, sizeof(lpTimerMgr->arrListTimer6) / sizeof(lpTimerMgr->arrListTimer6[0]));

	}
	return lpTimerMgr;
}

void DestroyTimerManager(TimerManager* pTimerManager)
{
	if (NULL == pTimerManager)
		return;

	pTimerManager->timerPool.Destroy();

	delete pTimerManager;
}

bool CreateTimer(TimerIdType timerId, TimerManager* pTimerManager, void (*OnTimer)(TimerIdType, void*), void* pParam, TimerMsType uDueTime, TimerMsType uPeriod)
{
	TimerNode* pTimer = NULL;
	if (NULL == OnTimer || NULL == pTimerManager)
		return false;
	pTimer = pTimerManager->timerPool.CreateObj(timerId);
	if (pTimer != NULL)
	{
		pTimer->periodMS = uPeriod;
		pTimer->OnTimer = OnTimer;
		pTimer->pParam = pParam;

		pTimer->thisFrameKilled = false;

		pTimer->expireMS = pTimerManager->currentTimeMS + uDueTime;

		AddTimer(pTimerManager, pTimer);
	}
	return true;
}

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId)
{
	ListTimer* pListTimer;
	if (NULL != pTimerManager && timerId != 0)
	{
		auto pTimer = pTimerManager->timerPool.FindObj(timerId);
		if (pTimer != nullptr)
		{
			pListTimer = &pTimer->listTimer;
			pListTimer->pPrev->pNext = pListTimer->pNext;
			pListTimer->pNext->pPrev = pListTimer->pPrev;
			pTimerManager->timerPool.ReleaseObj(timerId);
			pTimer->thisFrameKilled = true;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void TimerManager::Run()
{
#define INDEX(N) ((this->currentTimeMS >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)
	TimerMsType idx, timeMS;
	ListTimer listTimerExpire, * pListTimerExpire;
	TimerNode* pTimer;

	timeMS = UTimerGetCurrentTimeMS();
	while (timeMS >= currentTimeMS)
	{
		idx = currentTimeMS & TVR_MASK;
		if (!idx &&
			!CascadeTimer(this, this->arrListTimer2, INDEX(0)) &&
			!CascadeTimer(this, this->arrListTimer3, INDEX(1)) &&
			!CascadeTimer(this, this->arrListTimer4, INDEX(2)) &&
			!CascadeTimer(this, this->arrListTimer5, INDEX(3)) &&
			true)
		{
			CascadeTimer(this, this->arrListTimer6, INDEX(4));
		}
		pListTimerExpire = &listTimerExpire;
		ListTimerReplaceInit(&this->arrListTimer1[idx], pListTimerExpire);
		pListTimerExpire = pListTimerExpire->pNext;
		while (pListTimerExpire != &listTimerExpire)
		{
			pTimer = (TimerNode*)((uint8_t*)pListTimerExpire - offsetof(TimerNode, listTimer));
			pListTimerExpire = pListTimerExpire->pNext;
			auto timerId = pTimer->id;
			pTimer->OnTimer(pTimer->id, pTimer->pParam);
			if (pTimer->thisFrameKilled)
			{
				pTimer->thisFrameKilled = false;
				continue;
			}
			if (pTimer->periodMS != 0)
			{
				pTimer->expireMS = this->currentTimeMS + pTimer->periodMS;
				AddTimer(this, pTimer);
			}
			else
			{
				this->timerPool.ReleaseObj(timerId);
			}
		}
		this->currentTimeMS++;
	}
}
