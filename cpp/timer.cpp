#include "timer.h"

#include "timer_helper.h"

#include <stddef.h>
#include <stdlib.h>
#include <chrono>
#include <cassert>
#include <time.h>

static void AddTimer(TimerManager* pTimerManager, TimerNode* pTimer) {
    TimerMsType wheelIdx = 0;
    TimerMsType slotIdx = 0;

    TimerMsType qwExpires = pTimer->qwExpires;
    TimerMsType qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS_;

    if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (TIMER_WHEEL_COUNT))) {
        for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT; ++i) {
            if (qwDueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (i + 1))) {
                slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * i)) & TIMER_MASK;
                wheelIdx = i;
                break;
            }
        }
    } else if (qwDueTime < 0) {
        wheelIdx = 0;
        slotIdx = pTimerManager->qwCurrentTimeMS_ & TIMER_MASK;
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
static void CascadeTimer(TimerManager* pTimerManager, TimerMsType wheelIdx, TimerMsType slotIdx) {
    TimerNode* pTimer = pTimerManager->arrListTimerHead_[wheelIdx][slotIdx];
    TimerNode* pNext = nullptr;
    for (; pTimer != nullptr; pTimer = pNext) {
        pNext = pTimer->pNext;

        if (pTimer->bRunning) {
            AddTimer(pTimerManager, pTimer);
        } else {
            FreeObj(pTimer);
        }
    }
    pTimerManager->arrListTimerHead_[wheelIdx][slotIdx] = nullptr;
    pTimerManager->arrListTimerTail_[wheelIdx][slotIdx] = nullptr;
}

void TimerManager::Run() {
    TimerMsType idxExecutingSlotIdx = 0, idxNextWheelSlotIdx = 0;

    auto currTimeMS = UTimerGetCurrentTimeMS();
    TimerIdType timerId = 0;
    TimerNode* pTimer = nullptr;
    TimerNode* pNext = nullptr;
    while (currTimeMS >= this->qwCurrentTimeMS_) {
        idxExecutingSlotIdx = this->qwCurrentTimeMS_ & TIMER_MASK;

        idxNextWheelSlotIdx = idxExecutingSlotIdx;
        for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT - 1 && idxNextWheelSlotIdx == 0; ++i) {
            idxNextWheelSlotIdx = (this->qwCurrentTimeMS_ >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
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
                    FreeObj(pTimer);
                    this->mapTimers_.erase(timerId);
                }
            } else {
                FreeObj(pTimer);
            }
        }
        this->arrListTimerHead_[0][idxExecutingSlotIdx] = nullptr;
        this->arrListTimerTail_[0][idxExecutingSlotIdx] = nullptr;

        ++this->qwCurrentTimeMS_;
    }
}

TimerManager::TimerManager() {
    for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
        for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
            this->arrListTimerHead_[wheelIdx][slotIdx] = nullptr;
            this->arrListTimerTail_[wheelIdx][slotIdx] = nullptr;
        }
    }

    this->qwCurrentTimeMS_ = UTimerGetCurrentTimeMS();
}

TimerManager::~TimerManager() {
    TimerNode* pTimer = nullptr;
    TimerNode* pNext = nullptr;
    for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
        for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
            pTimer = this->arrListTimerHead_[wheelIdx][slotIdx];
            for (; pTimer != nullptr; pTimer = pNext) {
                pNext = pTimer->pNext;

                FreeObj(pTimer);
            }
        }
    }
}

bool TimerManager::CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod) {
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

    TimerNode* pTimer = AllocObj();
    if (pTimer == nullptr) {
        OnTimerError("CreateTimer AllocObj failed.");
        return false;
    }

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

bool TimerManager::KillTimer(TimerIdType timerId) {
    auto it = this->mapTimers_.find(timerId);
    if (it == this->mapTimers_.end()) {
        return false;
    }

    TimerNode* pTimer = it->second;
    if (!pTimer->bRunning) {
        return false;
    }

    pTimer->bRunning = false;
    this->mapTimers_.erase(it);

    return true;
}

void TimerManager::KillAllTimers() {
    for (auto it = this->mapTimers_.begin(); it != this->mapTimers_.end(); ++it) {
        it->second->bRunning = false;
    }
    this->mapTimers_.clear();
}
