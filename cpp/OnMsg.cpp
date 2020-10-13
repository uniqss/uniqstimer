#include "OnMsg.h"
#include "def.h"

uint64_t MainStartMS;

void OnTimer(TimerIdType timerId, void* pParam)
{
	const char* pszStr = (const char*)pParam;
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("%s. s:%llu ms:%llu str: %s\n", __FUNCTION__, s, ms, pszStr);
	g_MsgQueue.AddMessage(timerId, pszStr);
}

void checkTime(const TestCase& t, uint64_t currMS)
{
	t.triggerCount;
	MainStartMS;
	uint64_t ShouldMS = MainStartMS + t.dueTime + t.period * (t.triggerCount - 1);
	if (t.isNew)
	{
		ShouldMS += t.dueTimeNew + t.periodNew * (t.triggerCountNew - 1);
	}

	auto diff = ShouldMS > currMS ? ShouldMS - currMS : currMS - ShouldMS;
	if (diff > 10)
	{
		printf("%s check time failed. t.timerId:%llu t.msg:%s dueTime:%llu period:%llu, triggerCount:%d, kill:%d\n", __FUNCTION__, t.timerId, t.msg, t.dueTime, t.period, t.triggerCount, t.kill);
	}
	else {
		//printf("%s check time ok. t.timerId:%llu t.msg:%s dueTime:%llu period:%llu, triggerCount:%d, kill:%d", __FUNCTION__, t.timerId, t.msg, t.dueTime, t.period, t.triggerCount, t.kill);
	}
}

void OnMsg(uint64_t msgId, string msg)
{
	const char* pszStr = (const char*)msg.c_str();
	auto currMS = UTimerGetCurrentTimeMS();
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("%s. s:%llu ms:%llu str: %s\n", __FUNCTION__, s, ms, pszStr);

	bool bOk = false;

#if 0
	if (msgId == 10100)
	{
		bOk = KillTimer(pMgr, msgId);
		if (!bOk) printf("KillTimer failed. %d", __LINE__);
	}
	else if (msgId == 10200)
	{
		bOk = KillTimer(pMgr, msgId);
		if (!bOk) printf("KillTimer failed. %d", __LINE__);
	}
	else if (msgId == 20300)
	{
		bOk = KillTimer(pMgr, msgId);
		if (!bOk) printf("KillTimer failed. %d", __LINE__);
		bOk = CreateTimer(msgId, pMgr, OnTimer, (void*)"test kill timer and set timer on callback", 1000, 0);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
	else if (msgId == 20400)
	{
		bOk = KillTimer(pMgr, msgId);
		if (!bOk) printf("KillTimer failed. %d", __LINE__);
		auto r = rand() % 2;
		bOk = CreateTimer(msgId, pMgr, OnTimer, (void*)"test kill timer and set timer on callback", 1000, 2000);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
#endif
	auto it = g_TestCases.find(msgId);
	if (it != g_TestCases.end())
	{
		TestCase& t = it->second;
		if (t.isNew)
		{
			++t.triggerCountNew;
		}
		else
		{
			++t.triggerCount;
		}

		// 校验当前时间
		checkTime(t, currMS);

		if (t.kill < 0)
		{
			return;
		}
		--t.kill;
		if (t.kill == 0)
		{
			bOk = KillTimer(pMgr, msgId);
			if (!bOk) printf("KillTimer failed. %d timerId:%llu", __LINE__, msgId);
			if (t.bCreate)
			{
				bOk = CreateTimer(t.timerId, pMgr, OnTimer, (void*)t.msg, t.dueTimeNew, t.periodNew);
				if (!bOk) printf("CreateTimer failed. %d", __LINE__);
				t.isNew = true;
			}
		}

	}
}

