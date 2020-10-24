#include "main.h"

#include "timer_helper.h"

void OnTimerIII(TimerIdType timerId, void* pParam)
{
	const char* pszStr = (const char*)pParam;
	++FrameOnTimerCalled;
	++OnTimerCount;
	int64_t currTimeMS = UTimerGetCurrentTimeMS();

	printf("OnTimerIII timerId:%d pszStr:%s currMS:%d\n", timerId, pszStr, currTimeMS);
}

