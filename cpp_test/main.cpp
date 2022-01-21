#include <stdio.h>
#include "timer.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstddef>

#include <thread>

#include "fake_rand.h"

#include "main.h"

#include "timer_helper.h"


TimerIdType timerIdMotherCurr = timerIdMotherStart;

std::unique_ptr<TimerManager> pMgr;
std::unique_ptr<TimerManagerIII> pMgrIII;
bool bWorking = true;
bool bTerminateOk = false;
int RunExceed1MSCount = 0;
TimerMsType RunTotalUS = 0;
TimerMsType RunCount = 0;
TimerMsType RunAverageUS = 0;
TimerMsType OnTimerTotalUS = 0;
TimerMsType OnTimerCount = 0;
TimerMsType OnTimerCountSinceLastPrint = 0;
TimerMsType OnTimerAverageUS = 0;
uint64_t FrameOnTimerCalled = 0;


class TestRandTimerInfo {
   public:
    int state;  // 0默认 1created 2killed
    int triggeredCount;
    bool isRepeate;
    int triggeredCountRepeate;
    TimerMsType createTime;
    TimerMsType dueTime;
    TimerMsType period;

   public:
    TestRandTimerInfo() { Clear(); }

   public:
    void Clear() {
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

#include <gperftools/profiler.h>
int main(int argc, const char** argv) {
    ProfilerStart("uniqstimer.prof");
    arrTestRandTimerInfos.resize(timerIdRandCount);
    auto currMS = UTimerGetCurrentTimeMS();
    auto ms = currMS % 1000;
    auto s = currMS / 1000;
    printf("main start. s:%llu ms:%llu \n", s, ms);

    pMgr = std::unique_ptr<TimerManager>(new TimerManager());
    pMgrIII = std::unique_ptr<TimerManagerIII>(new TimerManagerIII());

#if 1
    std::thread t(LogicThread);
#else
    std::thread t(LogicThreadIII);
#endif
    t.detach();

    std::string input = "";
    auto lastPrintMS = UTimerGetCurrentTimeMS();
    while (true) {
        std::cin >> input;
        if (input == "exit" || input == "e") {
            bWorking = false;
            break;
        }
        if (input == "p" || input == "print") {
#if 1
            auto currPrintMS = UTimerGetCurrentTimeMS();
            printf("FrameOnTimerCalled:%llu RunExceed1MSCount:%d OnTimerCount:%llu OnTimerCountSinceLastPrint:%llu OnTimer/ms:%llu\n", FrameOnTimerCalled, RunExceed1MSCount,
                   OnTimerCount, OnTimerCount - OnTimerCountSinceLastPrint, (OnTimerCount - OnTimerCountSinceLastPrint) / (currPrintMS - lastPrintMS));
            OnTimerCountSinceLastPrint = OnTimerCount;
            lastPrintMS = currPrintMS;
#endif
            continue;
        }
    }

    while (!bTerminateOk) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }


    ProfilerStop();

    return 0;
}
