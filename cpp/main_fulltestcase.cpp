

#include "Timer.h"
#include<iostream>
#include<random>

#include "simplemessagequeue.h"
#include "OnMsg.h"

#include "def.h"

extern TimerManager* pMgr;
extern SimpleMessageQueue g_MsgQueue;
std::map<uint64_t, TestCase> g_TestCases;

void init_testcase()
{
#define UADD(id, dueTime, period, bKill, bCreate, dueTimeNew, periodNew) g_TestCases[id] = TestCase(id, #id, dueTime, period, bKill, bCreate, dueTimeNew, periodNew);
	UADD(TestTimerIdOnce1, TestMSRound1, 0, -1, false, 0, 0);
	UADD(TestTimerIdOnce2, TestMSRound2, 0, -1, false, 0, 0);
	UADD(TestTimerIdOnce3, TestMSRound3, 0, -1, false, 0, 0);

	UADD(TestTimerIdRepeateFirst1, TestMSRound1, 1000, -1, false, 0, 0);
	UADD(TestTimerIdRepeateFirst2, TestMSRound2, 1000, -1, false, 0, 0);
	UADD(TestTimerIdRepeateFirst3, TestMSRound3, 1000, -1, false, 0, 0);

	UADD(TestTimerIdRepeateRepeate1, 1000, TestMSRound1, -1, false, 0, 0);
	UADD(TestTimerIdRepeateRepeate2, 1000, TestMSRound2, -1, false, 0, 0);
	UADD(TestTimerIdRepeateRepeate3, 1000, TestMSRound3, -1, false, 0, 0);

	UADD(TestTimerIdOnceKill1, TestMSRound1, 1000, 1, false, 0, 0);
	UADD(TestTimerIdOnceKill2, TestMSRound2, 1000, 1, false, 0, 0);
	UADD(TestTimerIdOnceKill3, TestMSRound3, 1000, 1, false, 0, 0);

	UADD(TestTimerIdRepeateKillFirst1, TestMSRound1, 1000, 1, false, 0, 0);
	UADD(TestTimerIdRepeateKillFirst2, TestMSRound2, 1000, 1, false, 0, 0);
	UADD(TestTimerIdRepeateKillFirst3, TestMSRound3, 1000, 1, false, 0, 0);

	UADD(TestTimerIdRepeateKillRepeate1, 1000, TestMSRound1, 1, false, 0, 0);
	UADD(TestTimerIdRepeateKillRepeate2, 1000, TestMSRound2, 1, false, 0, 0);
	UADD(TestTimerIdRepeateKillRepeate3, 1000, TestMSRound3, 1, false, 0, 0);

	UADD(TestTimerIdOnceKillCreate11, TestMSRound1, 0, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdOnceKillCreate12, TestMSRound1, 0, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdOnceKillCreate13, TestMSRound1, 0, 1, true, TestMSRound3, 0);
	UADD(TestTimerIdOnceKillCreate21, TestMSRound2, 0, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdOnceKillCreate22, TestMSRound2, 0, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdOnceKillCreate23, TestMSRound2, 0, 1, true, TestMSRound3, 0);
	UADD(TestTimerIdOnceKillCreate31, TestMSRound3, 0, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdOnceKillCreate32, TestMSRound3, 0, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdOnceKillCreate33, TestMSRound3, 0, 1, true, TestMSRound3, 0);

	UADD(TestTimerIdOnceKillCreateRepeateFirst11, TestMSRound1, 0, 1, true, TestMSRound1, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst12, TestMSRound1, 0, 1, true, TestMSRound2, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst13, TestMSRound1, 0, 1, true, TestMSRound3, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst21, TestMSRound2, 0, 1, true, TestMSRound1, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst22, TestMSRound2, 0, 1, true, TestMSRound2, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst23, TestMSRound2, 0, 1, true, TestMSRound3, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst31, TestMSRound3, 0, 1, true, TestMSRound1, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst32, TestMSRound3, 0, 1, true, TestMSRound2, 1000);
	UADD(TestTimerIdOnceKillCreateRepeateFirst33, TestMSRound3, 0, 1, true, TestMSRound3, 1000);

	UADD(TestTimerIdOnceKillCreateRepeateRepeate11, TestMSRound1, 0, 1, true, 1000, TestMSRound1);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate12, TestMSRound1, 0, 1, true, 1000, TestMSRound2);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate13, TestMSRound1, 0, 1, true, 1000, TestMSRound3);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate21, TestMSRound2, 0, 1, true, 1000, TestMSRound1);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate22, TestMSRound2, 0, 1, true, 1000, TestMSRound2);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate23, TestMSRound2, 0, 1, true, 1000, TestMSRound3);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate31, TestMSRound3, 0, 1, true, 1000, TestMSRound1);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate32, TestMSRound3, 0, 1, true, 1000, TestMSRound2);
	UADD(TestTimerIdOnceKillCreateRepeateRepeate33, TestMSRound3, 0, 1, true, 1000, TestMSRound3);

	UADD(TestTimerIdRepeateFirstKillCreateOnce11, TestMSRound1, 1000, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce12, TestMSRound1, 1000, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce13, TestMSRound1, 1000, 1, true, TestMSRound3, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce21, TestMSRound2, 1000, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce22, TestMSRound2, 1000, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce23, TestMSRound2, 1000, 1, true, TestMSRound3, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce31, TestMSRound3, 1000, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce32, TestMSRound3, 1000, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdRepeateFirstKillCreateOnce33, TestMSRound3, 1000, 1, true, TestMSRound3, 0);

	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst11, TestMSRound1, 5000, 1, true, TestMSRound1, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst12, TestMSRound1, 5000, 1, true, TestMSRound2, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst13, TestMSRound1, 5000, 1, true, TestMSRound3, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst21, TestMSRound2, 5000, 1, true, TestMSRound1, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst22, TestMSRound2, 5000, 1, true, TestMSRound2, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst23, TestMSRound2, 5000, 1, true, TestMSRound3, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst31, TestMSRound3, 5000, 1, true, TestMSRound1, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst32, TestMSRound3, 5000, 1, true, TestMSRound2, 6000);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateFirst33, TestMSRound3, 5000, 1, true, TestMSRound3, 6000);

	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate11, TestMSRound1, 5000, 1, true, 6000, TestMSRound1);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate12, TestMSRound1, 5000, 1, true, 6000, TestMSRound2);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate13, TestMSRound1, 5000, 1, true, 6000, TestMSRound3);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate21, TestMSRound2, 5000, 1, true, 6000, TestMSRound1);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate22, TestMSRound2, 5000, 1, true, 6000, TestMSRound2);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate23, TestMSRound2, 5000, 1, true, 6000, TestMSRound3);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate31, TestMSRound3, 5000, 1, true, 6000, TestMSRound1);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate32, TestMSRound3, 5000, 1, true, 6000, TestMSRound2);
	UADD(TestTimerIdRepeateFirstKillCreateRepeateRepeate33, TestMSRound3, 5000, 1, true, 6000, TestMSRound3);

	UADD(TestTimerIdRepeateRepeateKillCreateOnce11, 5000, TestMSRound1, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce12, 5000, TestMSRound1, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce13, 5000, TestMSRound1, 1, true, TestMSRound3, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce21, 5000, TestMSRound2, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce22, 5000, TestMSRound2, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce23, 5000, TestMSRound2, 1, true, TestMSRound3, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce31, 5000, TestMSRound3, 1, true, TestMSRound1, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce32, 5000, TestMSRound3, 1, true, TestMSRound2, 0);
	UADD(TestTimerIdRepeateRepeateKillCreateOnce33, 5000, TestMSRound3, 1, true, TestMSRound3, 0);

	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce11, 5000, TestMSRound1, 1, true, TestMSRound1, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce12, 5000, TestMSRound1, 1, true, TestMSRound2, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce13, 5000, TestMSRound1, 1, true, TestMSRound3, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce21, 5000, TestMSRound2, 1, true, TestMSRound1, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce22, 5000, TestMSRound2, 1, true, TestMSRound2, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce23, 5000, TestMSRound2, 1, true, TestMSRound3, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce31, 5000, TestMSRound3, 1, true, TestMSRound1, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce32, 5000, TestMSRound3, 1, true, TestMSRound2, 7000);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateOnce33, 5000, TestMSRound3, 1, true, TestMSRound3, 7000);

	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate11, 5000, TestMSRound1, 1, true, 7000, TestMSRound1);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate12, 5000, TestMSRound1, 1, true, 7000, TestMSRound2);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate13, 5000, TestMSRound1, 1, true, 7000, TestMSRound3);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate21, 5000, TestMSRound2, 1, true, 7000, TestMSRound1);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate22, 5000, TestMSRound2, 1, true, 7000, TestMSRound2);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate23, 5000, TestMSRound2, 1, true, 7000, TestMSRound3);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate31, 5000, TestMSRound3, 1, true, 7000, TestMSRound1);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate32, 5000, TestMSRound3, 1, true, 7000, TestMSRound2);
	UADD(TestTimerIdRepeateRepeateKillCreateRepeateRepeate33, 5000, TestMSRound3, 1, true, 7000, TestMSRound3);
}


void main_fulltestcase()
{
	init_testcase();

	bool bOk = false;
//#define POK if (!bOk) printf("CreateTimer failed. %d", __LINE__);

	for (auto it : g_TestCases)
	{
		const TestCase& t = it.second;
		if (1 == 1)
		{
			if (t.timerId > 6||t.timerId <= 3)
			{
				continue;
			}
		}
		bOk = CreateTimer(t.timerId, pMgr, OnTimer, (void*)t.msg, t.dueTime, t.period);
		if (!bOk) printf("CreateTimer failed. %d", __LINE__);
	}
}