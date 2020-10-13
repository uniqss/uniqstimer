#include <stdio.h>
#include "Timer.h"
#include<iostream>
#include<random>

#include "simplemessagequeue.h"
#include "OnMsg.h"

TimerManager* pMgr;
SimpleMessageQueue g_MsgQueue;


void OnTimerKill(TimerIdType timerId, void* pParam)
{
	const char* pszStr = (const char*)pParam;
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("%s. s:%llu ms:%llu str: %s\n", __FUNCTION__, s, ms, pszStr);
	bool bOk = false;
	bOk = KillTimer(pMgr, timerId);
	if (!bOk) printf("KillTimer failed. %d", __LINE__);
}

std::atomic<bool> MainThreadWorking = true;


void ExitThread()
{
	std::string input = "";
	while (true)
	{
		std::cin >> input;
		if (input == "exit" || input == "e")
		{
			break;
		}
	}
	MainThreadWorking = false;
}

extern void main_fulltestcase();

#define TIMERCOUNT 1000000
#define RANDTIMERCOUNT 100000
int main(void)
{
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("main start. s:%llu ms:%llu \n", s, ms);

	MainStartMS = currMS;

	pMgr = CreateTimerManager(true);
	bool bOk = false;

	main_fulltestcase();

#if 0
	bOk = CreateTimer(TimerIdType(10100), pMgr, OnTimer, (void*)"test kill timer and set timer on callback", 100, 1000);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(10200), pMgr, OnTimer, (void*)"test kill timer and set timer on callback", 300, 0);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(20300), pMgr, OnTimer, (void*)"test kill timer and set timer on callback", 500, 2000);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	bOk = CreateTimer(TimerIdType(20400), pMgr, OnTimer, (void*)"test kill timer and set timer on callback", 800, 0);
	if (!bOk) printf("CreateTimer failed. %d", __LINE__);
#endif

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
	srand(UTimerGetCurrentTimeMS());
	for (auto i = 0; i < RANDTIMERCOUNT; i++)
	{
		auto t1 = rand() % 1000 + 100;
		auto t2 = rand() % 500 + 500;
		bOk = CreateTimer(TimerIdType(1), pMgr, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", t1, t2);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
#endif

	std::thread e(ExitThread);
	e.detach();

	while (MainThreadWorking)
	{
		SimpleMessage msg;
		while (g_MsgQueue.PopMessage(msg))
		{
			OnMsg(msg.msgId, msg.msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
		if (!bOk) printf("KillTimer failed. %d %llu", __LINE__, i);
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

	DestroyTimerManager(pMgr);
	return 0;
}