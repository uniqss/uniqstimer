#include <stdio.h>
#include "Timer.h"
#include<iostream>
#include<random>
#include<thread>

TimerManager* pMgr;

void OnTimer(TimerIdType timerId, void* pParam)
{
	const char* pszStr = (const char*)pParam;
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("OnTimer. s:%llu ms:%llu timerId:%llu str: %s\n", s, ms, timerId, pszStr);
	auto randKill = rand() % 100;
	if (randKill < 10)
	{
		KillTimer(pMgr, timerId);
	}
}

#define TIMERCOUNT 1000000
#define RANDTIMERCOUNT 100000

bool bWorking = true;
bool bTerminateOK = false;

void LogicThread()
{
	pMgr = CreateTimerManager(true);

	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("main start. s:%llu ms:%llu \n", s, ms);

	bool bOk = false;

#if 1
	bOk = CreateTimer(TimerIdType(1), pMgr, OnTimer, (void*)"this is first 200 ms then 200 ms repeated timer", 200, 200);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(2), pMgr, OnTimer, (void*)"this is first 1000 ms then 1000 ms repeated timer", 1000, 1000);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(3), pMgr, OnTimer, (void*)"this is first 20000 ms then 20000 ms repeated timer", 20000, 20000);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(4), pMgr, OnTimer, (void*)"this is first 200 ms then 200 ms repeated timer", 200, 0);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(5), pMgr, OnTimer, (void*)"this is first 1000 ms then 1000 ms repeated timer", 1000, 0);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(6), pMgr, OnTimer, (void*)"this is first 20000 ms then 20000 ms repeated timer", 20000, 0);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);
#endif

#if 0
	for (auto i = 0; i < 1000000; i++)
	{
		bOk = CreateTimer(TimerIdType(i), pMgr, OnTimer, (void*)"this is first 1000 ms then 1000 ms repeated timer", 1000, 1000);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
#endif

	while (bWorking)
	{
		pMgr->Run();
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}

	DestroyTimerManager(pMgr);
	bTerminateOK = true;
}

int main(void)
{
	bool bOk = false;

	srand(UTimerGetCurrentTimeMS());

#if 0
	for (TimerIdType i = 1; i < TIMERCOUNT; i++)
	{
		bOk = CreateTimer(TimerIdType(i), pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
#endif

#if 0
	bOk = CreateTimer(TimerIdType(1), pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);
#endif

#if 0
	for (auto i = 0; i < RANDTIMERCOUNT; i++)
	{
		auto t1 = rand() % 1000 + 100;
		auto t2 = rand() % 500 + 500;
		bOk = CreateTimer(TimerIdType(1), pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", t1, t2);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
#endif

	std::thread t(LogicThread);
	t.detach();

	std::string input = "";
	while (true)
	{
		std::cin >> input;
		if (input == "exit" || input == "e")
		{
			bWorking = false;
			break;
		}
	}

#if 0
	for (TimerIdType i = 1; i < TIMERCOUNT; i++)
	{
		bOk = KillTimer(pMgr, TimerIdType(i));
		if (!bOk) printf("KillTimer failed. %d %llu", __LINE__, i);
	}
#endif

#if 0
	for (auto i = 1; i < RANDTIMERCOUNT; i++)
	{
		bOk = KillTimer(pMgr, TimerIdType(i));
		if (!bOk) printf("KillTimer failed. %d %d", __LINE__, i);
	}
#endif

#if 0
	bOk = KillTimer(pMgr, TimerIdType(1));
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = KillTimer(pMgr, TimerIdType(1));
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = KillTimer(pMgr, TimerIdType(10000));
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);
#endif

	while (!bTerminateOK)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}