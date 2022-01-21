#pragma once

#include "timer.h"
#include <memory>

const TimerIdType timerIdMotherMother = 1;
const TimerIdType timerIdMotherStart = 1000000;
const TimerIdType timerIdMotherCount = 5000;
const TimerIdType timerIdMotherStop = timerIdMotherStart + timerIdMotherCount;

const TimerIdType timerIdRandStart = 10000000;
const TimerIdType timerIdRandCount = 100000;

extern std::unique_ptr<TimerManager<>> pMgr;
extern std::unique_ptr<TimerManager<>> pMgrIII;

extern bool bWorking;
extern bool bTerminateOk;
extern int RunExceed1MSCount;
extern TimerMsType RunTotalUS;
extern TimerMsType RunCount;
extern TimerMsType RunAverageUS;
extern TimerMsType OnTimerTotalUS;
extern TimerMsType OnTimerCount;
extern TimerMsType OnTimerCountSinceLastPrint;
extern TimerMsType OnTimerAverageUS;
extern uint64_t FrameOnTimerCalled;

void OnTimerIIIPressureTest(TimerIdType timerId, void* pParam);
void OnTimerPressureTest(TimerIdType timerId, void* pParam);
void LogicThread();
void LogicThreadIII();
