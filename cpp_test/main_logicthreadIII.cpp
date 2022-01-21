#include "fake_rand.h"
#include <stdlib.h>
#include "timer.h"
#include "main.h"

#include "timer_helper.h"

#include <thread>

void LogicThreadIII() {
    srand((unsigned)UTimerGetCurrentTimeMS());
    FakeRandInit();

    for (TimerIdType i = 1; i <= 200000; ++i) {
        pMgrIII->CreateTimer(i, OnTimerIIIPressureTest, (void*)"timer LV III timer test pressure test", (i % 1000) + 1, 1000);
    }

    int64_t lastMS = UTimerGetCurrentTimeMS();
    int64_t currMS = 0;

    int64_t tmpDiffSum = 0;
    int lessCount = 0;
    int exceedCount = 0;
    while (bWorking) {
        FrameOnTimerCalled = 0;

        currMS = UTimerGetCurrentTimeMS();
        while (currMS - lastMS >= 100) {
            int64_t tmpUSStart = UTimerGetCurrentTimeUS();

            pMgrIII->Run();
            ++RunCount;
            lastMS += 100;

            int64_t tmpUSEnd = UTimerGetCurrentTimeUS();
            int64_t tmpUSDiff = (tmpUSEnd - tmpUSStart) / 1000000;
            if (tmpUSDiff > 0) {
                // printf("%llu\n", tmpUSDiff);
                ++RunExceed1MSCount;
                ++exceedCount;
                tmpDiffSum += tmpUSDiff;
            } else {
                // printf(".");
                ++lessCount;
            }
            if (lessCount + exceedCount == 10) {
                printf("%d|%lld ", exceedCount, tmpDiffSum);
                lessCount = 0;
                exceedCount = 0;
                tmpDiffSum = 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    bTerminateOk = true;
}
