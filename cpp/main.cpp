#include <stdio.h>
#include "Timer.h"
#include<iostream>

void OnTimer(void* pParam)
{
	const char* pszStr = (const char*)pParam;
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("OnTimer. s:%llu ms:%llu str: %s\n", s, ms, pszStr);
}

#define TIMERCOUNT 512
TimerNode* pTimers[1024] = { nullptr, };
int main(void)
{
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("main start. s:%llu ms:%llu \n", s, ms);

	for (auto i = 0; i < TIMERCOUNT; i++)
	{
		pTimers[i] = nullptr;
	}
	TimerManager* pMgr;
	pMgr = CreateTimerManager();
	pTimers[0] = CreateTimer(pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
	pTimers[1] = CreateTimer(pMgr, OnTimer, (void*)"this is first 500 ms then 0 ms timer", 500, 0);
	pTimers[2] = CreateTimer(pMgr, OnTimer, (void*)"this is first 2000 ms then 1000 ms repeated timer", 2000, 1000);
	pTimers[3] = CreateTimer(pMgr, OnTimer, (void*)"this is first 3000 ms then 0 ms timer", 3000, 0);
	pTimers[3] = CreateTimer(pMgr, OnTimer, (void*)"this is first 0 ms then 4000 ms timer", 0, 4000);

	std::string input = "";
	while (true)
	{
		std::cin >> input;
		if (input == "exit" || input == "e")
		{
			break;
		}
	}

	for (size_t i = 0; i < TIMERCOUNT; i++)
	{
		if (pTimers[i] != nullptr)
		{
			DeleteTimer(pMgr, pTimers[i]);
			pTimers[i] = nullptr;
		}
	}
	DestroyTimerManager(pMgr);
	return 0;
}