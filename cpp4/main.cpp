#include <stdio.h>
#include "Timer.h"
#include<iostream>
#include<vector>
#include<string>

#include<thread>

TimerManager* pMgr;
bool bWorking = true;
bool bTerminateOk = false;


TimerIdType timerIdRandStart = 10000000;
TimerIdType timerIdRandCount = 30000;

class TestRandTimerInfo
{
public:
	int state;// 0д╛хо 1created 2killed
	int triggeredCount;
	bool isImmediate;
	bool isRepeate;
	int triggeredCountRepeate;
	TimerMsType createTime;
	TimerMsType dueTime;
	TimerMsType period;
public:
	TestRandTimerInfo()
	{
		Clear();
	}
public:
	void Clear()
	{
		state = 0;
		triggeredCount = 0;
		isImmediate = false;
		isRepeate = false;
		triggeredCountRepeate = 0;
		createTime = 0;
		dueTime = 0;
		period = 0;
	}
};

std::vector<TestRandTimerInfo> arrTestRandTimerInfos;

void OnTimer(TimerIdType timerId, void* pParam)
{
	const char* pszStr = (const char*)pParam;
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("OnTimer. timerId:%llu s:%llu ms:%llu str: %s\n", timerId, s, ms, pszStr);
	bool bOk = false;

	auto randKill = rand() % 1000;
	if (randKill < 500)
	{
#if 0
		bOk = KillTimer(pMgr, timerId);
		if (!bOk)
		{
			printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
		}
		bOk = KillTimer(pMgr, timerId);
		if (bOk)
		{
			printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
		}
		bOk = KillTimer(pMgr, timerId);
		if (bOk)
		{
			printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
		}
#endif
#if 0
		bOk = KillTimer(pMgr, timerId);
		if (!bOk)
		{
			printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
		}
		auto randCreate = rand() % 1000;
		if (randCreate < 750)
		{
			bOk = CreateTimer(pMgr, timerId, OnTimer, (void*)"this is created by OnTimer", 300, 300);
			if (!bOk)
			{
				printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
			}
		}
#endif
	}
#if 0
	if (timerId < 1000001)
	{
		bOk = KillTimer(pMgr, timerId);
		if (!bOk)
		{
			printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
		}
		TimerMsType t1 = 0;
		TimerMsType t2 = 0;
		// 0-255 256-65536 65536-16777216
		if (timerId % 3 == 0)
		{
			t1 = rand() % 256;
			t2 = rand() % 256;
		}
		else if (timerId % 3 == 1)
		{
			t1 = rand() % 65536;
			t2 = rand() % 65536;
		}
		else
		{
			t1 = rand() % 131072;
			t2 = rand() % 131072;
		}
		bOk = CreateTimer(pMgr, timerId, OnTimer, (void*)"", t1, t2);
		if (!bOk)
		{
			printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
		}
	}
#endif
#if 1
	if (timerId >= 1000001 && timerId < 1000100)
	{
		// kill create rand
		auto randTimerIdIdx = (rand() % timerIdRandCount);
		auto randTimerId = randTimerIdIdx + timerIdRandStart;

		static int RunningTimersCount = 100;
		auto randKill = rand() % 1000;
		auto& rInfo = arrTestRandTimerInfos[randTimerIdIdx];
		auto& rState = rInfo.state;
		if (RunningTimersCount > 100 && randKill < 500)
		{
			// 0д╛хо 1created 2killed
			if (rState == 1)
			{
				bOk = KillTimer(pMgr, randTimerId);
				if (!bOk)
				{
					throw std::logic_error("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
					//printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
				}
				// set data
				RunningTimersCount--;
				rInfo.Clear();
				rState = 2;
				bOk = KillTimer(pMgr, randTimerId);
				if (bOk)
				{
					throw std::logic_error("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
			}
			else if (rState == 2 || rState == 0)
			{
				bOk = KillTimer(pMgr, randTimerId);
				if (bOk)
				{
					throw std::logic_error("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
				rState = 2;
			}
		}
		TimerIdType randCreate = rand() % 10000;
		if (RunningTimersCount < timerIdRandCount && randCreate < 8000)
		{
			auto randImmediate = (rand() % 2) > 0;
			auto randRepeate = (rand() % 2) > 0;
			if (randImmediate)
			{
				randRepeate = true;
			}
			TimerMsType t1 = randImmediate ? 0 : ((rand() % (131072 + 6)) + 1);
			TimerMsType t2 = randRepeate ? ((rand() % (131072 + 6)) + 1) : 0;

			// 0д╛хо 1created 2killed
			if (rState == 1)
			{
				bOk = CreateTimer(pMgr, randTimerId, OnTimer, (void*)"", t1, t2);
				if (bOk)
				{
					throw std::logic_error("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
			}
			else
			{
				bOk = CreateTimer(pMgr, randTimerId, OnTimer, (void*)"", t1, t2);
				if (!bOk)
				{
					throw std::logic_error("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
				// set data
				RunningTimersCount++;
				rState = 1;
				rInfo.triggeredCount = 0;
				rInfo.isImmediate = randImmediate;
				rInfo.isRepeate = randRepeate;
				rInfo.createTime = currMS;
				rInfo.dueTime = t1;
				rInfo.period = t2;
				bOk = CreateTimer(pMgr, randTimerId, OnTimer, (void*)"", t1, t2);
				if (bOk)
				{
					throw std::logic_error("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
			}
		}
	}
#endif
#if 1
	if (timerId >= timerIdRandStart)
	{
		TimerMsType timerIdIdx = timerId - timerIdRandStart;
		auto& rInfo = arrTestRandTimerInfos[timerIdIdx];
		TimerMsType shouldTimeMS = rInfo.createTime;
		if (rInfo.dueTime == 0)
		{
			++rInfo.triggeredCountRepeate;
		}
		else if (rInfo.isRepeate && rInfo.triggeredCount > 0)
		{
			++rInfo.triggeredCountRepeate;
		}
		else
		{
			++rInfo.triggeredCount;
		}

		shouldTimeMS += rInfo.dueTime * rInfo.triggeredCount + rInfo.period * rInfo.triggeredCountRepeate;

		TimerMsType diff = shouldTimeMS > currMS ? shouldTimeMS - currMS : currMS - shouldTimeMS;
		if (diff > 100)
		{
			throw std::logic_error("timer time check error");
		}
	}
#endif
}

void LogicThread()
{
	srand((unsigned)UTimerGetCurrentTimeMS());
	//CreateTimer(pMgr, 1, OnTimer, (void*)"test", 1, 1);
#if 0
	for (size_t i = 1; i <= 60000; i++)
	{
		auto t = i % 66666 + 1;
		CreateTimer(pMgr, i, OnTimer, (void*)"test", t, t);
	}
#endif
#if 1
	for (size_t i = 1000001; i <= 1000100; i++)
	{
		CreateTimer(pMgr, i, OnTimer, (void*)"test", 1000, 1000);
	}
#endif
#if 0
	CreateTimer(pMgr, 1001, OnTimer, (void*)"this is first 100 ms then 100 ms repeated timer", 100, 100);
	CreateTimer(pMgr, 2001, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
	CreateTimer(pMgr, 1002, OnTimer, (void*)"this is first 500 ms then 0 ms timer", 500, 0);
	CreateTimer(pMgr, 2002, OnTimer, (void*)"this is first 500 ms then 100 ms timer", 500, 100);
	CreateTimer(pMgr, 1003, OnTimer, (void*)"this is first 600 ms then 0 ms timer", 600, 0);
	CreateTimer(pMgr, 2003, OnTimer, (void*)"this is first 600 ms then 100 ms timer", 600, 100);
	CreateTimer(pMgr, 1004, OnTimer, (void*)"this is first 2000 ms then 1000 ms repeated timer", 2000, 1000);
	CreateTimer(pMgr, 2004, OnTimer, (void*)"this is first 3000 ms then 0 ms timer", 3000, 0);
	CreateTimer(pMgr, 1005, OnTimer, (void*)"this is first 0 ms then 4000 ms timer", 0, 4000);
	CreateTimer(pMgr, 2005, OnTimer, (void*)"this is first 0 ms then 4000 ms timer", 0, 4000);
#endif

#if 0
	CreateTimer(pMgr, 100, OnTimer, (void*)"100", 100, 100);
#endif
#if 0
	CreateTimer(pMgr, 300, OnTimer, (void*)"300", 300, 300);
	CreateTimer(pMgr, 600, OnTimer, (void*)"600", 600, 600);
#endif

	while (bWorking)
	{
		pMgr->Run();
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}
	bTerminateOk = true;
}

#define TIMERCOUNT 512
int main(void)
{
	arrTestRandTimerInfos.resize(timerIdRandCount);
#ifdef UNIQS_DEBUG_TIMER
	DebugDiffTimeMs = 0;
#if 0
	DebugDiffTimeMs = UTimerGetCurrentTimeMS();
	//DebugDiffTimeMs -= 100 + 100 * (1 << TIMER_BITS_PER_WHEEL) + 100 * (1 << (2 * TIMER_BITS_PER_WHEEL)) + 100 * (1 << (3 * TIMER_BITS_PER_WHEEL));
	DebugDiffTimeMs -= 100  + 100 * (1 << TIMER_BITS_PER_WHEEL);
#endif
#endif
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("main start. s:%llu ms:%llu \n", s, ms);

	pMgr = CreateTimerManager();

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

	while (!bTerminateOk)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	DestroyTimerManager(pMgr);
	return 0;
}