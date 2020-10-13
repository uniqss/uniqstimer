#pragma once

#include <stdio.h>
#include "Timer.h"
#include<iostream>
#include<random>

#include "simplemessagequeue.h"

void OnMsg(uint64_t msgId, string msg);

void OnTimer(TimerIdType timerId, void* pParam);

extern TimerManager* pMgr;
extern SimpleMessageQueue g_MsgQueue;

extern uint64_t MainStartMS;
