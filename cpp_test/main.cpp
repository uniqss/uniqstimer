#include <stdio.h>
#include "timer.h"
#include <iostream>
#include <vector>
#include <string>

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

TimerMsType UTimerGetCurrentTimeUS(void) {
#if 1
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    // char buff[100];
    // strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
    // printf("Current time: %s.%09ld UTC\n", buff, ts.tv_nsec);
    TimerMsType ret = (TimerMsType)ts.tv_sec * 1000000000 + ts.tv_nsec;
    return ret;
#else
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_us = std::chrono::duration_cast<std::chrono::nanoseconds>(time_now.time_since_epoch());
    return (TimerMsType)duration_in_us.count();
#endif
}

int main(int argc, const char** argv) {
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
    while (true) {
        std::cin >> input;
        if (input == "exit" || input == "e") {
            bWorking = false;
            break;
        }
        if (input == "p" || input == "print") {
#if 1
            printf("FrameOnTimerCalled:%llu RunExceed1MSCount:%d RunAverageUS:%llu OnTimerTotalUS:%llu OnTimerCount:%llu\n", FrameOnTimerCalled, RunExceed1MSCount,
                   RunAverageUS, OnTimerTotalUS, OnTimerCount);
#endif
            continue;
        }
    }

    while (!bTerminateOk) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    return 0;
}
