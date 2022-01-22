#include <cstddef>

#include "fake_rand.h"
#include <stdlib.h>
#include "timer.h"
#include "main.h"

#include "timer_helper.h"

#include <thread>
#include <cstdio>

#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
#include <Windows.h>
static void usleep(int64_t usec) {
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
#else
#include <unistd.h>
#endif  // defined(WIN32) || defined(_WIN32) || defined(WINDOWS)


void LogicThread(TimerIdType timerCount, int64_t tickMicroSeconds, int64_t usleepOnRun, int64_t usleepOnNotRun) {
    srand((unsigned)UTimerGetCurrentTimeMS());
    FakeRandInit();

    for (TimerIdType i = 1; i <= timerCount; ++i) {
        pMgr->CreateTimer(i, OnTimerPressureTest, (void*)"timer LV I timer test pressure test", (i % 1000) + 1, 1000);
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
            pMgr->Run();
            ++RunCount;
            lastUS += tickMicroSeconds;

            int64_t tmpUSEnd = UTimerGetCurrentTimeUS();
            RunTotalTime += tmpUSEnd - currUS;
            if (tmpUSEnd - currUS > 1000) {
                ++RunExceed1MSCount;
                ++exceedCount;
                tmpDiffSum += tmpUSEnd - currUS;
            } else {
                ++lessCount;
            }
            if ((lessCount + exceedCount) * tickMicroSeconds >= 1000000) {
                printf("%d|%lld|%lld ", exceedCount, tmpDiffSum, RunTotalTime / RunCount);
                fflush(stdout);

                lessCount = 0;
                exceedCount = 0;
                tmpDiffSum = 0;
            }
            if (usleepOnRun > 0) usleep(usleepOnRun);
        } else {
            if (usleepOnNotRun > 0) usleep(usleepOnNotRun);
        }
    }
    bTerminateOk = true;
}
