#include <stdio.h>
#include "Timer.h"
#include<iostream>
#include<vector>
#include<string>

#include<thread>

#include "glog_helper.h"
#pragma comment(lib, "glogd.lib")

TimerManager* pMgr;
bool bWorking = true;
bool bTerminateOk = false;

TimerIdType timerIdMotherStart = 1000000;
TimerIdType timerIdMotherCount = 1000;
TimerIdType timerIdMotherStop = timerIdMotherStart + timerIdMotherCount;
TimerIdType timerIdRandStart = 10000000;
TimerIdType timerIdRandCount = 1000000;

class TestRandTimerInfo
{
public:
	int state;// 0默认 1created 2killed
	int triggeredCount;
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

#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "OnTimer timerId:" << timerId << " pszStr:" << pszStr << " currMS:" << currMS;
#endif

#if 0
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("OnTimer. timerId:%llu s:%llu ms:%llu str: %s\n", timerId, s, ms, pszStr);
#endif
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
	if (timerId < timerIdMotherStart)
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
	static int RunningTimersCount = 100;
	if (timerId >= timerIdMotherStart && timerId <= timerIdMotherStop)
	{
		// kill create rand
		auto randTimerIdIdx = (rand() % timerIdRandCount);
		auto randTimerId = randTimerIdIdx + timerIdRandStart;

		TimerIdType __randKill = rand() % 10000;
#if 1
		// 最低20% 最高85%
		TimerIdType __randKillPercent = 2000 + ((TimerIdType)6500 * RunningTimersCount / timerIdRandCount);
#else
		TimerIdType __randKillPercent = 5000;
#endif
		timerIdRandCount;
		auto& rInfo = arrTestRandTimerInfos[randTimerIdIdx];
		auto& rState = rInfo.state;
		if (RunningTimersCount > 100 && __randKill < __randKillPercent)
		{
#ifdef UNIQS_LOG_EVERYTHING
			LOG(INFO) << "OnTimer rand kill timerId:" << timerId << " rState:" << rState << " randTimerId:" << randTimerId << " currMS:" << currMS;
#endif
			// 0默认 1created 2killed
			if (rState == 1)
			{
				bOk = KillTimer(pMgr, randTimerId);
				if (!bOk)
				{
					OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
					//printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
				}
				// set data
				RunningTimersCount--;
				rInfo.Clear();
				rState = 2;
				bOk = KillTimer(pMgr, randTimerId);
				if (bOk)
				{
					OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
			}
			else if (rState == 2 || rState == 0)
			{
#ifdef UNIQS_LOG_EVERYTHING
				LOG(INFO) << "OnTimer rand kill timerId:" << timerId << " randTimerId:" << randTimerId << " currMS:" << currMS;
#endif
				bOk = KillTimer(pMgr, randTimerId);
				if (bOk)
				{
					OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
				rState = 2;
			}
		}
		TimerIdType randCreate = rand() % 10000;
		if (RunningTimersCount < timerIdRandCount && randCreate < 8000)
		{
			auto randRepeate = (rand() % 2) > 0;
			TimerMsType t1 = ((rand() % (131072 + 6)) + 1);
			TimerMsType t2 = randRepeate ? ((rand() % (131072 + 6)) + 1) : 0;

#ifdef UNIQS_LOG_EVERYTHING
			LOG(INFO) << "OnTimer rand create timerId:" << timerId << " rState:" << rState << " randTimerId:" << randTimerId <<
				" currMS:" << currMS << " t1:" << t1 << " t2:" << t2;
#endif
			// 0默认 1created 2killed
			if (rState == 1)
			{
				bOk = CreateTimer(pMgr, randTimerId, OnTimer, (void*)"", t1, t2);
				if (bOk)
				{
					OnTimerError("OnTimer CreateTimer should fail but succeeded. randTimerId:" + std::to_string(randTimerId));
				}
			}
			else
			{
				bOk = CreateTimer(pMgr, randTimerId, OnTimer, (void*)"", t1, t2);
				if (!bOk)
				{
					OnTimerError("OnTimer CreateTimer should succeed but failed. randTimerId:" + std::to_string(randTimerId));
				}
				// set data
				RunningTimersCount++;
				rState = 1;
				rInfo.triggeredCount = 0;
				rInfo.isRepeate = randRepeate;
				rInfo.createTime = currMS;
				rInfo.dueTime = t1;
				rInfo.period = t2;
				bOk = CreateTimer(pMgr, randTimerId, OnTimer, (void*)"", t1, t2);
				if (bOk)
				{
					OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
				}
			}
		}
	}

#if 1
	static TimerMsType lastTimeMS = UTimerGetCurrentTimeMS();
	TimerMsType diff = currMS - lastTimeMS;
	static TimerMsType OnTimerTriggered = 0;
	++OnTimerTriggered;
	if (diff > 1000)
	{
		printf("RunningTimersCount:%d OnTimerTriggered:%llu\n", RunningTimersCount, OnTimerTriggered);
		lastTimeMS = currMS;
	}
#endif
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

		TimerMsType __diff = shouldTimeMS > currMS ? shouldTimeMS - currMS : currMS - shouldTimeMS;
		if (__diff > 100)
		{
			OnTimerError("timer time check error");
		}

		if (!rInfo.isRepeate)
		{
			rInfo.state = 0;
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
	for (TimerIdType i = timerIdMotherStart; i <= timerIdMotherStop; i++)
	{
		CreateTimer(pMgr, i, OnTimer, (void*)"test", 1000, 1000);
	}
#endif
#if 1
	/*
	CreateTimer(pMgr, 1001, OnTimer, (void*)"this is first 100 ms then 100 ms repeated timer", 100, 100);
	CreateTimer(pMgr, 2001, OnTimer, (void*)"this is first 100 ms then 1000 ms repeated timer", 100, 1000);
	CreateTimer(pMgr, 2001, OnTimer, (void*)"this is first 300 ms then 1000 ms repeated timer", 300, 1000);
	*/
	/*
	CreateTimer(pMgr, 1002, OnTimer, (void*)"this is first 500 ms then 0 ms timer", 500, 0);
	CreateTimer(pMgr, 2002, OnTimer, (void*)"this is first 500 ms then 100 ms timer", 500, 100);
	CreateTimer(pMgr, 1003, OnTimer, (void*)"this is first 600 ms then 0 ms timer", 600, 0);
	CreateTimer(pMgr, 2003, OnTimer, (void*)"this is first 600 ms then 100 ms timer", 600, 100);
	CreateTimer(pMgr, 1004, OnTimer, (void*)"this is first 2000 ms then 1000 ms repeated timer", 2000, 1000);
	CreateTimer(pMgr, 2004, OnTimer, (void*)"this is first 3000 ms then 0 ms timer", 3000, 0);
	CreateTimer(pMgr, 1005, OnTimer, (void*)"this is first 0 ms then 4000 ms timer", 0, 4000);
	CreateTimer(pMgr, 2005, OnTimer, (void*)"this is first 0 ms then 4000 ms timer", 0, 4000);
	*/
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
int main(int argc, const char** argv)
{
	init_glog(argv[0], "./logs");
	arrTestRandTimerInfos.resize(timerIdRandCount);
#ifdef UNIQS_DEBUG_TIMER
	DebugDiffTimeMs = 0;
#if 1
	DebugDiffTimeMs = UTimerGetCurrentTimeMS();
	//DebugDiffTimeMs -= 100 + 100 * (1 << TIMER_BITS_PER_WHEEL) + 100 * (1 << (2 * TIMER_BITS_PER_WHEEL)) + 100 * (1 << (3 * TIMER_BITS_PER_WHEEL));
	DebugDiffTimeMs -= 100 + 100 * (1 << TIMER_BITS_PER_WHEEL);
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

	google::ShutdownGoogleLogging();
	return 0;
}