#include <cstddef>

#include "fake_rand.h"
#include <stdlib.h>
#include "timer.h"
#include "main.h"

#include "timer_helper.h"

#include <thread>
#include <cstdio>
#include <unistd.h>

void LogicThreadIII() {
    srand((unsigned)UTimerGetCurrentTimeMS());
    FakeRandInit();

    for (TimerIdType i = 1; i <= 300000; ++i) {
        pMgrIII->CreateTimer(i, OnTimerIIIPressureTest, (void*)"timer LV III timer test pressure test", (i % 1000) + 1, 1000);
    }

    int64_t lastUS = UTimerGetCurrentTimeUS();
    int64_t currUS = 0;

    int64_t tmpDiffSum = 0;
    int lessCount = 0;
    int exceedCount = 0;
    while (bWorking) {
        FrameOnTimerCalled = 0;

        currUS = UTimerGetCurrentTimeUS();
        if (currUS > lastUS) {
            pMgrIII->Run();
            ++RunCount;
            lastUS += 100000;

            int64_t tmpUSEnd = UTimerGetCurrentTimeUS();
            RunTotalUS += tmpUSEnd - currUS;
            if (tmpUSEnd > currUS + 1000) {
                ++RunExceed1MSCount;
                ++exceedCount;
                tmpDiffSum += tmpUSEnd - currUS;
            } else {
                ++lessCount;
            }
            if (lessCount + exceedCount >= 10) {
                printf("%d|%lld|%lld ", exceedCount, tmpDiffSum, RunTotalUS/RunCount);
                fflush(stdout);

                lessCount = 0;
                exceedCount = 0;
                tmpDiffSum = 0;
            }
#if 0
        std::this_thread::sleep_for(std::chrono::microseconds(10));
#else
            usleep(50);
#endif
        } else {
#if 0
        std::this_thread::sleep_for(std::chrono::microseconds(10));
#else
            usleep(50);
#endif
        }
    }
    bTerminateOk = true;
}
