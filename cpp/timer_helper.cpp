#include "timer_helper.h"

#include<time.h>
#include<stdexcept>

int UniqsTimerAllocCalled = 0;
int UniqsTimerFreeCalled = 0;

TimerNode* __pFreeTimerHeadMem;
int UniqsTimerFreeCount = 0;
const int UNIQS_TIMER_CACHE_MAX = 4096;
const int UNIQS_TIMER_CACHE_DELETE = UNIQS_TIMER_CACHE_MAX / 2;

TimerNodeIII* __pFreeTimerHeadMemIII;
int UniqsTimerFreeCountIII = 0;
const int UNIQS_TIMER_CACHE_MAXIII = 4096;
const int UNIQS_TIMER_CACHE_DELETEIII = UNIQS_TIMER_CACHE_MAXIII / 2;

TimerNode* AllocObj()
{
	UniqsTimerAllocCalled++;

	if (__pFreeTimerHeadMem != nullptr)
	{
		UniqsTimerFreeCount--;
		TimerNode* pTimer = __pFreeTimerHeadMem;
		__pFreeTimerHeadMem = pTimer->pNext;
		pTimer->pNext = nullptr;
		return pTimer;
	}
	auto ret = new TimerNode();
	return ret;
}
void FreeObj(TimerNode* pTimer)
{
	UniqsTimerFreeCalled++;
	UniqsTimerFreeCount++;
	if (__pFreeTimerHeadMem == nullptr)
	{
		__pFreeTimerHeadMem = pTimer;
		__pFreeTimerHeadMem->pNext = nullptr;
	}
	else
	{
		pTimer->pNext = __pFreeTimerHeadMem;
		__pFreeTimerHeadMem = pTimer;
	}


	if (UniqsTimerFreeCount > UNIQS_TIMER_CACHE_MAX)
	{
		TimerNode* pDelete = __pFreeTimerHeadMem;
		for (int i = 0; i < UNIQS_TIMER_CACHE_DELETE; i++)
		{
			__pFreeTimerHeadMem = pDelete->pNext;

			// free memory
			delete pDelete;

			pDelete = __pFreeTimerHeadMem;
		}
		UniqsTimerFreeCount -= UNIQS_TIMER_CACHE_DELETE;
	}
}

TimerNodeIII* AllocObjIII()
{
	if (__pFreeTimerHeadMemIII != nullptr)
	{
		UniqsTimerFreeCountIII--;
		TimerNodeIII* pTimer = __pFreeTimerHeadMemIII;
		__pFreeTimerHeadMemIII = pTimer->pNext;
		pTimer->pNext = nullptr;
		return pTimer;
	}
	auto ret = new TimerNodeIII();
	return ret;
}
void FreeObjIII(TimerNodeIII* pTimer)
{
	UniqsTimerFreeCountIII++;
	if (__pFreeTimerHeadMemIII == nullptr)
	{
		__pFreeTimerHeadMemIII = pTimer;
		__pFreeTimerHeadMemIII->pNext = nullptr;
	}
	else
	{
		pTimer->pNext = __pFreeTimerHeadMemIII;
		__pFreeTimerHeadMemIII = pTimer;
	}


	if (UniqsTimerFreeCountIII > UNIQS_TIMER_CACHE_MAXIII)
	{
		TimerNodeIII* pDelete = __pFreeTimerHeadMemIII;
		for (int i = 0; i < UNIQS_TIMER_CACHE_DELETEIII; i++)
		{
			__pFreeTimerHeadMemIII = pDelete->pNext;

			// free memory
			delete pDelete;

			pDelete = __pFreeTimerHeadMemIII;
		}
		UniqsTimerFreeCountIII -= UNIQS_TIMER_CACHE_DELETEIII;
	}
}

TimerMsType UTimerGetCurrentTimeMS(void)
{
	return clock();
}

void OnTimerError(const std::string& err)
{
	throw std::logic_error("OnTimerError" + err);
}
