#include "fake_rand.h"
#include<stdlib.h>
#include "Timer.h"
#include "main.h"
#include "glog_helper.h"

#include "timer_helper.h"

void LogicThread()
{
	srand((unsigned)UTimerGetCurrentTimeMS());
	FakeRandInit();

	//CreateTimer(pMgr, 1, OnTimer, (void*)"test", 1, 1);
#if 0
	pMgr->CreateTimer(timerIdMotherMother, OnTimer, (void*)"mother", 100, 100);
#endif

	pMgrIII->CreateTimer(1, OnTimerIII, (void*)"timer LV III timer test", 500, 500);

	int64_t beginUS = 0;
	int64_t endUS = 0;
	int64_t diffUS = 0;

	int64_t lastMS = 0;
	int64_t currMS = 0;
	while (bWorking)
	{
		++RunCount;
		FrameOnTimerCalled = 0;
		beginUS = UTimerGetCurrentTimeUS();
		pMgr->Run();


		currMS = UTimerGetCurrentTimeMS();
		if (currMS - lastMS > 100)
		{
			//printf("pre pMgrIII->Run currMS:%llu\n", currMS);
			pMgrIII->Run();
			lastMS = currMS;
		}


		endUS = UTimerGetCurrentTimeUS();
		diffUS = endUS - beginUS;
		RunTotalUS += diffUS;


		//if (500 - diffUS / 1000 > 0)
		if (diffUS < 1000000)
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(1000000 - diffUS));
		}
		else
		{
			//OnTimerError("OnTimer exceeded 1ms");
			++RunExceed1MSCount;
		}
	}
	bTerminateOk = true;
}
