#include <stdio.h>
#include "Timer.h"
#include<iostream>

void OnTimer(void* pParam)
{
	const char* pszStr = (const char*)pParam;
	auto currMS = UTimerGetCurrentTimeMS();
	//auto ms = currMS % 1000;
	//auto s = currMS / 1000;
	//printf("OnTimer. s:%llu ms:%llu str: %s\n", s, ms, pszStr);
}

#define TIMERCOUNT 1000000
int main(void)
{
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("main start. s:%llu ms:%llu \n", s, ms);

	TimerManager* pMgr;
	pMgr = CreateTimerManager(true);
	bool bOk = false;
	for (TimerIdType i = 1; i < TIMERCOUNT; i++)
	{
		bOk = CreateTimer(TimerIdType(i), pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}

#if 0
	bOk = CreateTimer(TimerIdType(1), pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);
#endif

	std::string input = "";
	while (true)
	{
		std::cin >> input;
		if (input == "exit" || input == "e")
		{
			break;
		}
	}

	for (TimerIdType i = 1; i < TIMERCOUNT; i++)
	{
		bOk = KillTimer(pMgr, TimerIdType(i));
		if (!bOk) printf("KillTimer failed. %d %llu", __LINE__, i);
	}
	bOk = KillTimer(pMgr, TimerIdType(1));
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = KillTimer(pMgr, TimerIdType(1));
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = KillTimer(pMgr, TimerIdType(10000));
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	DestroyTimerManager(pMgr);
	return 0;
}