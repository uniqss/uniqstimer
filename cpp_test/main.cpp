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

std::unique_ptr<TimerManager<>> pMgr;
std::unique_ptr<TimerManager<>> pMgrIII;
bool bWorking = true;
bool bTerminateOk = false;
int RunExceed1MSCount = 0;
TimerMsType RunTotalTime = 0;
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


#if defined(WIN32) || defined(_WIN32) || defined(WINDOWS)
#define GPERFTOOLS_PROFIE 0
#else
#define GPERFTOOLS_PROFIE 1
#endif
#if GPERFTOOLS_PROFIE
#include <gperftools/profiler.h>
#endif
int main(int argc, const char** argv) {
#if GPERFTOOLS_PROFIE
    ProfilerStart("uniqstimer.prof");
#endif
    arrTestRandTimerInfos.resize(timerIdRandCount);
    auto currMS = UTimerGetCurrentTimeMS();
    auto ms = currMS % 1000;
    auto s = currMS / 1000;
    printf("main start. s:%llu ms:%llu \n", s, ms);

    int64_t timerTickMs = 1;
    if (argc >= 2) {
        timerTickMs = atoll(argv[1]);
        if (timerTickMs <= 0 || timerTickMs >= 1000) {
            printf("timerTickMs[%lld] not valid, using default 1ms\n", timerTickMs);
            timerTickMs = 1;
        }
    }
    printf("timerTickMs:%lld \n", timerTickMs);
    pMgr = std::unique_ptr<TimerManager<>>(new TimerManager<>(timerTickMs));
    std::thread t([&]() { LogicThread(1000000, timerTickMs * 1000, 0, 1000); });
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }


#if GPERFTOOLS_PROFIE
    ProfilerStop();
#endif

    return 0;
}
