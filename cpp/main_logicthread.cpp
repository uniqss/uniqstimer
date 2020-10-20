#include "fake_rand.h"
#include<stdlib.h>
#include "Timer.h"
#include "main.h"
#include "glog_helper.h"

void LogicThread()
{
	srand((unsigned)UTimerGetCurrentTimeMS());
	FakeRandInit();

	//CreateTimer(pMgr, 1, OnTimer, (void*)"test", 1, 1);
#if 1
	CreateTimer(pMgr, timerIdMotherMother, OnTimer, (void*)"mother", 100, 100);
#endif

#if 0
	CreateTimer(pMgr, 100, OnTimer, (void*)"100", 100, 100);
#endif
#if 0
	CreateTimer(pMgr, 300, OnTimer, (void*)"300", 300, 300);
	CreateTimer(pMgr, 600, OnTimer, (void*)"600", 600, 600);
#endif

	int64_t beginUS = 0;
	int64_t endUS = 0;
	int64_t diffUS = 0;
	while (bWorking)
	{
		++RunCount;
		FrameOnTimerCalled = 0;
		beginUS = UTimerGetCurrentTimeUS();
		pMgr->Run();

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
