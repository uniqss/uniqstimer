#include <stdio.h>
#include "Timer.h"
#include <iostream>
#include <vector>
#include <string>

#include <thread>

#include "fake_rand.h"

#include "glog_helper.h"
#include "main.h"

#include "timer_helper.h"


#if defined(WIN32) || defined(WINDOWS) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glogd.lib")
#else
#pragma comment(lib, "glog.lib")
#endif
#else
#endif

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

void OnTimer(TimerIdType timerId, void* pParam) {
    const char* pszStr = (const char*)pParam;
    ++FrameOnTimerCalled;
    ++OnTimerCount;
    TimerMsType startUS = UTimerGetCurrentTimeUS();
    TimerMsType currMS = UTimerGetCurrentTimeMS();

#ifdef UNIQS_LOG_EVERYTHING
    LOG(INFO) << "OnTimer timerId:" << timerId << " pszStr:" << pszStr << " currMS:" << currMS;
#endif

#if 0
	auto ms = currMS % 1000;
	auto s = currMS / 1000;
	printf("OnTimer. timerId:%llu s:%llu ms:%llu str: %s\n", timerId, s, ms, pszStr);
#endif
    bool bOk = false;

#if 1
    if (timerId == timerIdMotherMother) {
        if (timerIdMotherCurr < timerIdMotherStop) {
            for (size_t i = 0; i < 100; i++) {
                pMgr->CreateTimer(timerIdMotherCurr, OnTimer, (void*)"mother", 500, 500);
                timerIdMotherCurr++;
            }
        } else {
            pMgr->KillTimer(timerIdMotherMother);
        }
    }
    static int RunningTimersCount = 100;
    if (timerId >= timerIdMotherStart && timerId <= timerIdMotherStop) {
        // kill create rand
        auto fakeRand = FakeRand();
        auto randTimerIdIdx = (fakeRand % timerIdRandCount);
        auto randTimerId = randTimerIdIdx + timerIdRandStart;

        TimerIdType __randKill = FakeRand() % 10000;
#if 0
		// 最低20% 最高85%
		TimerIdType __randKillPercent = 2000 + ((TimerIdType)6500 * RunningTimersCount / timerIdRandCount);
#else
        TimerIdType __randKillPercent = 5000;
#endif
        timerIdRandCount;
        auto& rInfo = arrTestRandTimerInfos[randTimerIdIdx];
        auto& rState = rInfo.state;
        if (false && RunningTimersCount > 100 && __randKill < __randKillPercent) {
#ifdef UNIQS_LOG_EVERYTHING
            LOG(INFO) << "OnTimer rand kill timerId:" << timerId << " rState:" << rState << " randTimerId:" << randTimerId << " currMS:" << currMS;
#endif
            // 0默认 1created 2killed
            if (rState == 1) {
                bOk = pMgr->KillTimer(randTimerId);
                if (!bOk) {
                    OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
                    // printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
                }
                // set data
                RunningTimersCount--;
                rInfo.Clear();
                rState = 2;
                bOk = pMgr->KillTimer(randTimerId);
                if (bOk) {
                    OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
                }
            } else if (rState == 2 || rState == 0) {
#ifdef UNIQS_LOG_EVERYTHING
                LOG(INFO) << "OnTimer rand kill timerId:" << timerId << " randTimerId:" << randTimerId << " currMS:" << currMS;
#endif
                bOk = pMgr->KillTimer(randTimerId);
                if (bOk) {
                    OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
                }
                rState = 2;
            }
        }
        TimerIdType randCreate = FakeRand() % 10000;
        if (RunningTimersCount < timerIdRandCount && randCreate < 8000) {
            // 性能测试，避免创建大量小于128毫秒的定时器，128-256落到第一个轮的概率还是很大。
#if 0
			auto randRepeate = (FakeRand() % 2) > 0;
			TimerMsType t1 = ((FakeRand() % (131072 + 6)) + 128);
			TimerMsType t2 = randRepeate ? ((FakeRand() % (131072 + 6)) + 128) : 0;
#else
            TimerMsType t1 = 500;
            TimerMsType t2 = 500;
            auto randRepeate = true;
#endif

#ifdef UNIQS_LOG_EVERYTHING
            LOG(INFO) << "OnTimer rand create timerId:" << timerId << " rState:" << rState << " randTimerId:" << randTimerId << " currMS:" << currMS << " t1:" << t1
                      << " t2:" << t2;
#endif
            // 0默认 1created 2killed
            if (rState == 1) {
                bOk = pMgr->CreateTimer(randTimerId, OnTimer, (void*)"", t1, t2);
                if (bOk) {
                    OnTimerError("OnTimer CreateTimer should fail but succeeded. randTimerId:" + std::to_string(randTimerId));
                }
            } else {
                bOk = pMgr->CreateTimer(randTimerId, OnTimer, (void*)"", t1, t2);
                if (!bOk) {
                    OnTimerError("OnTimer CreateTimer should succeed but failed. randTimerId:" + std::to_string(randTimerId));
                }
                // set data
                RunningTimersCount++;
                rState = 1;
                rInfo.triggeredCount = 0;
                rInfo.isRepeate = randRepeate;
                rInfo.createTime = currMS;
                rInfo.dueTime = t1;
                rInfo.period = t2;
                bOk = pMgr->CreateTimer(randTimerId, OnTimer, (void*)"", t1, t2);
                if (bOk) {
                    OnTimerError("OnTimer KillTimer failed randTimerId:" + std::to_string(randTimerId));
                }
            }
        }
    }

#if 1
    static TimerMsType lastTimeMS = UTimerGetCurrentTimeMS();
    TimerMsType diff = currMS - lastTimeMS;
    static TimerMsType OnTimerTriggered = 0;
    ++OnTimerTriggered;
    if (diff > 1000) {
        extern int UniqsTimerAllocCalled;
        extern int UniqsTimerFreeCalled;
        extern int UniqsTimerFreeCount;
        RunAverageUS = RunTotalUS / RunCount;
        OnTimerAverageUS = OnTimerTotalUS / OnTimerCount;
        auto percent = (OnTimerTotalUS)*100 / RunTotalUS;
        auto av = OnTimerCount / RunCount;

#if 0
		printf("Timers:%d FOT:%llu OnTimerTriggered:%llu Alloced:%d Freeed:%d FreeM:%d Over1MS:%d RunAvUS:%llu OnTimer:%llu|%llu|%llu|%llu\n"
			, RunningTimersCount, FrameOnTimerCalled, OnTimerTriggered, UniqsTimerAllocCalled, UniqsTimerFreeCalled, UniqsTimerFreeCount, RunExceed1MSCount, RunAverageUS
			, OnTimerTotalUS, OnTimerCount, OnTimerAverageUS, percent);
#endif
#if 0
		printf("Timers:%d Run:%llu|%llu|%llu OnTimer:%llu|%llu|%llu OnTimer/Run:%llu\%% OnTimerCount/RunCount:%llu\n",
			RunningTimersCount, RunTotalUS, RunCount, RunAverageUS, OnTimerTotalUS, OnTimerCount, OnTimerAverageUS, percent, av);
#endif

        lastTimeMS = currMS;
    }
#endif
#endif
#if 1
    if (timerId >= timerIdRandStart) {
        TimerMsType timerIdIdx = timerId - timerIdRandStart;
        auto& rInfo = arrTestRandTimerInfos[timerIdIdx];
        TimerMsType shouldTimeMS = rInfo.createTime;
        if (rInfo.dueTime == 0) {
            ++rInfo.triggeredCountRepeate;
        } else if (rInfo.isRepeate && rInfo.triggeredCount > 0) {
            ++rInfo.triggeredCountRepeate;
        } else {
            ++rInfo.triggeredCount;
        }

        shouldTimeMS += rInfo.dueTime * rInfo.triggeredCount + rInfo.period * rInfo.triggeredCountRepeate;

        TimerMsType __diff = shouldTimeMS > currMS ? shouldTimeMS - currMS : currMS - shouldTimeMS;
        if (__diff > 1000) {
            OnTimerError("timer time check error");
        }

        if (!rInfo.isRepeate) {
            rInfo.state = 0;
        }
    }
#endif
    TimerMsType endUS = UTimerGetCurrentTimeUS();
    OnTimerTotalUS += endUS - startUS;
}

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

#define TIMERCOUNT 512
int main(int argc, const char** argv) {
    init_glog(argv[0], "./logs");
    arrTestRandTimerInfos.resize(timerIdRandCount);
    auto currMS = UTimerGetCurrentTimeMS();
    auto ms = currMS % 1000;
    auto s = currMS / 1000;
    printf("main start. s:%llu ms:%llu \n", s, ms);

    pMgr = std::unique_ptr<TimerManager>(new TimerManager());
    pMgrIII = std::unique_ptr<TimerManagerIII>(new TimerManagerIII());

    std::thread t(LogicThread);
    t.detach();

    std::string input = "";
    while (true) {
        std::cin >> input;
        if (input == "exit" || input == "e") {
            bWorking = false;
            break;
        }
    }

    while (!bTerminateOk) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    google::ShutdownGoogleLogging();
    return 0;
}