#include "timer_mem.h"

TimerNode* __pFreeTimerHeadMem;
int UniqsTimerAllocCalled = 0;
int UniqsTimerFreeCalled = 0;
int UniqsTimerFreeCount = 0;
const int UNIQS_TIMER_CACHE_MAX = 4096;
const int UNIQS_TIMER_CACHE_DELETE = UNIQS_TIMER_CACHE_MAX / 2;

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
