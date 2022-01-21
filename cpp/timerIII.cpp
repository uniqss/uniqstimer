#include "timerIII.h"

#include "timer_helper.h"

#include <stddef.h>
#include <stdlib.h>
#include <chrono>
#include <cassert>
#include <time.h>

static void AddTimer(TimerManagerIII* pTimerManager, TimerNodeIII* pTimer) {
    TimerMsTypeIII wheelIdx = 0;
    TimerMsTypeIII slotIdx = 0;

    TimerMsTypeIII qwExpires = pTimer->qwExpires;
    TimerMsTypeIII qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS_;

    if (qwDueTime < TimerMsTypeIII(1) << (TIMER_BITS_PER_WHEELIII * (TIMER_WHEEL_COUNTIII))) {
        for (TimerMsTypeIII i = 0; i < TIMER_WHEEL_COUNT; ++i) {
            if (qwDueTime < TimerMsTypeIII(1) << (TIMER_BITS_PER_WHEELIII * (i + 1))) {
                slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEELIII * i)) & TIMER_MASKIII;
                wheelIdx = i;
                break;
            }
        }
    } else if (qwDueTime < 0) {
        wheelIdx = 0;
        slotIdx = pTimerManager->qwCurrentTimeMS_ & TIMER_MASKIII;
    } else {
        OnTimerError("AddTimer this should not happen");
        return;
    }

    if (pTimerManager->arrListTimerTail_[wheelIdx][slotIdx] == nullptr) {
        pTimerManager->arrListTimerHead_[wheelIdx][slotIdx] = pTimer;
        pTimerManager->arrListTimerTail_[wheelIdx][slotIdx] = pTimer;
        pTimer->pNext = nullptr;
    } else {
        pTimerManager->arrListTimerTail_[wheelIdx][slotIdx]->pNext = pTimer;
        pTimer->pNext = nullptr;
        pTimerManager->arrListTimerTail_[wheelIdx][slotIdx] = pTimer;
    }
}

// 循环该slot里的所有节点，重新AddTimer到管理器中
static void CascadeTimer(TimerManagerIII* pTimerManager, TimerMsTypeIII wheelIdx, TimerMsTypeIII slotIdx) {
    TimerNodeIII* pTimer = pTimerManager->arrListTimerHead_[wheelIdx][slotIdx];
    TimerNodeIII* pNext = nullptr;
    for (; pTimer != nullptr; pTimer = pNext) {
        pNext = pTimer->pNext;

        if (pTimer->bRunning) {
            AddTimer(pTimerManager, pTimer);
        } else {
            FreeObjIII(pTimer);
        }
    }
    pTimerManager->arrListTimerHead_[wheelIdx][slotIdx] = nullptr;
    pTimerManager->arrListTimerTail_[wheelIdx][slotIdx] = nullptr;
}

void TimerManagerIII::Run() {
    TimerMsType idxExecutingSlotIdx = 0, idxNextWheelSlotIdx = 0;

    auto currTimeMS = UTimerGetCurrentTimeMS() / qwTickWheelMS_;
    TimerIdType timerId = 0;
    TimerNodeIII* pTimer = nullptr;
    TimerNodeIII* pNext = nullptr;
    while (currTimeMS >= this->qwCurrentTimeMS_) {
        idxExecutingSlotIdx = this->qwCurrentTimeMS_ & TIMER_MASKIII;

        idxNextWheelSlotIdx = idxExecutingSlotIdx;
        for (TimerMsType i = 0; i < TIMER_WHEEL_COUNTIII - 1 && idxNextWheelSlotIdx == 0; ++i) {
            idxNextWheelSlotIdx = (this->qwCurrentTimeMS_ >> ((i + 1) * TIMER_BITS_PER_WHEELIII)) & TIMER_MASKIII;
            CascadeTimer(this, i + 1, idxNextWheelSlotIdx);
        }

        pTimer = this->arrListTimerHead_[0][idxExecutingSlotIdx];
        for (; pTimer != nullptr; pTimer = pNext) {
            pNext = pTimer->pNext;

            timerId = pTimer->qwTimerId;
            if (pTimer->bRunning) {
                pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam);
                if (pTimer->qwPeriod != 0) {
                    pTimer->qwExpires = this->qwCurrentTimeMS_ + pTimer->qwPeriod;
                    AddTimer(this, pTimer);
                } else {
                    FreeObjIII(pTimer);
                    this->mapTimers_.erase(timerId);
                }
            } else {
                FreeObjIII(pTimer);
            }
        }
        this->arrListTimerHead_[0][idxExecutingSlotIdx] = nullptr;
        this->arrListTimerTail_[0][idxExecutingSlotIdx] = nullptr;

        ++this->qwCurrentTimeMS_;
    }
}

TimerManagerIII::TimerManagerIII(TimerMsTypeIII TIMER_MS_COUNTIII_) : qwTickWheelMS_(TIMER_MS_COUNTIII_) {
    for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNTIII; ++wheelIdx) {
        for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEELIII; ++slotIdx) {
            this->arrListTimerHead_[wheelIdx][slotIdx] = nullptr;
            this->arrListTimerTail_[wheelIdx][slotIdx] = nullptr;
        }
    }

    this->qwCurrentTimeMS_ = UTimerGetCurrentTimeMS() / qwTickWheelMS_;
}

TimerManagerIII::~TimerManagerIII() {
    TimerNodeIII* pTimer = nullptr;
    TimerNodeIII* pNext = nullptr;
    for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNTIII; ++wheelIdx) {
        for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEELIII; ++slotIdx) {
            pTimer = this->arrListTimerHead_[wheelIdx][slotIdx];
            for (; pTimer != nullptr; pTimer = pNext) {
                pNext = pTimer->pNext;

                FreeObjIII(pTimer);
            }
        }
    }
}

bool TimerManagerIII::CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsTypeIII qwDueTime, TimerMsTypeIII qwPeriod) {
    if (NULL == timerFn) return false;

    // 两者都为0,无意义
    if (qwDueTime == 0 && qwPeriod == 0) {
        return false;
    }

    // 如果创建重复性定时器，又设置触发时间为0,就直接把时间设置为重复时间
    if (qwDueTime == 0) {
        return false;
    }

    auto it = this->mapTimers_.find(timerId);
    if (it != this->mapTimers_.end()) {
        return false;
    }

    TimerNodeIII* pTimer = AllocObjIII();
    if (pTimer == nullptr) {
        OnTimerError("CreateTimer AllocObj failed.");
        return false;
    }

    qwDueTime /= qwTickWheelMS_;
    qwPeriod /= qwTickWheelMS_;

    pTimer->qwPeriod = qwPeriod;
    pTimer->timerFn = timerFn;
    pTimer->pParam = pParam;
    pTimer->qwTimerId = timerId;

    pTimer->bRunning = true;

    pTimer->qwExpires = this->qwCurrentTimeMS_ + qwDueTime;
    AddTimer(this, pTimer);
    this->mapTimers_[timerId] = pTimer;

    return true;
}

bool TimerManagerIII::KillTimer(TimerIdType timerId) {
    auto it = this->mapTimers_.find(timerId);
    if (it == this->mapTimers_.end()) {
        return false;
    }

    TimerNodeIII* pTimer = it->second;
    if (!pTimer->bRunning) {
        return false;
    }

    pTimer->bRunning = false;
    this->mapTimers_.erase(it);

    return true;
}

void TimerManagerIII::KillAllTimers() {
    for (auto it = this->mapTimers_.begin(); it != this->mapTimers_.end(); ++it) {
        it->second->bRunning = false;
    }
    this->mapTimers_.clear();
}
