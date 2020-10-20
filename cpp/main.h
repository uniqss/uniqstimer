#pragma once

const TimerIdType timerIdMotherMother = 1;
const TimerIdType timerIdMotherStart = 1000000;
const TimerIdType timerIdMotherCount = 5000;
const TimerIdType timerIdMotherStop = timerIdMotherStart + timerIdMotherCount;

const TimerIdType timerIdRandStart = 10000000;
const TimerIdType timerIdRandCount = 3000000;

extern TimerIdType timerIdMotherCurr;

extern TimerManager* pMgr;
extern bool bWorking;
extern bool bTerminateOk;
extern int RunExceed1MSCount;
extern TimerMsType RunTotalUS;
extern TimerMsType RunCount;
extern TimerMsType RunAverageUS;
extern TimerMsType OnTimerTotalUS;
extern TimerMsType OnTimerCount;
extern TimerMsType OnTimerAverageUS;
extern uint64_t FrameOnTimerCalled;


void OnTimer(TimerIdType timerId, void* pParam, TimerMsType currMS);
void LogicThread();
TimerMsType UTimerGetCurrentTimeUS(void);
