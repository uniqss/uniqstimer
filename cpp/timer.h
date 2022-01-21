#pragma once

#include <unordered_map>
#include <stdint.h>

#define TimerIdType int64_t
#define TimerMsType int64_t

#define TIMER_BITS_PER_WHEEL 10
#define TIMER_SLOT_COUNT_PER_WHEEL 1 << TIMER_BITS_PER_WHEEL
#define TIMER_WHEEL_COUNT 4
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

class TimerNode {
   public:
    TimerNode* pNext;
    TimerIdType qwTimerId;
    TimerMsType qwExpires;  //
    TimerMsType qwPeriod;
    void (*timerFn)(TimerIdType, void*);
    void* pParam;
    bool bRunning;
};

#include "timer_helper.h"

template <class NodeAllocator = TimerNodeAllocator>
class TimerManager {
    NodeAllocator allocator_;

    void AddTimer(TimerNode* pTimer) {
        TimerMsType wheelIdx = 0;
        TimerMsType slotIdx = 0;

        TimerMsType qwExpires = pTimer->qwExpires;
        TimerMsType qwDueTime = qwExpires - this->qwCurrentTimeMS_;

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
            slotIdx = this->qwCurrentTimeMS_ & TIMER_MASK;
        } else {
            OnTimerError("AddTimer this should not happen");
            return;
        }

        if (arrListTimerTail_[wheelIdx][slotIdx] == nullptr) {
            arrListTimerHead_[wheelIdx][slotIdx] = pTimer;
            arrListTimerTail_[wheelIdx][slotIdx] = pTimer;
            pTimer->pNext = nullptr;
        } else {
            arrListTimerTail_[wheelIdx][slotIdx]->pNext = pTimer;
            pTimer->pNext = nullptr;
            arrListTimerTail_[wheelIdx][slotIdx] = pTimer;
        }
    }

    void CascadeTimer(TimerMsType wheelIdx, TimerMsType slotIdx) {
        TimerNode* pTimer = arrListTimerHead_[wheelIdx][slotIdx];
        TimerNode* pNext = nullptr;
        for (; pTimer != nullptr; pTimer = pNext) {
            pNext = pTimer->pNext;

            if (pTimer->bRunning) {
                AddTimer(pTimer);
            } else {
                allocator_.FreeObj(pTimer);
            }
        }
        arrListTimerHead_[wheelIdx][slotIdx] = nullptr;
        arrListTimerTail_[wheelIdx][slotIdx] = nullptr;
    }

   public:
    TimerManager(TimerMsType qwTickOneSlotMS = 1) : qwTickOneSlotMS_(qwTickOneSlotMS) {
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                this->arrListTimerHead_[wheelIdx][slotIdx] = nullptr;
                this->arrListTimerTail_[wheelIdx][slotIdx] = nullptr;
            }
        }

        this->qwCurrentTimeMS_ = UTimerGetCurrentTimeMS() / qwTickOneSlotMS_;
    }
    ~TimerManager() {
        TimerNode* pTimer = nullptr;
        TimerNode* pNext = nullptr;
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                pTimer = this->arrListTimerHead_[wheelIdx][slotIdx];
                for (; pTimer != nullptr; pTimer = pNext) {
                    pNext = pTimer->pNext;

                    allocator_.FreeObj(pTimer);
                }
            }
        }
    }

    // qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
    bool CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod) {
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

        TimerNode* pTimer = allocator_.AllocObj();
        if (pTimer == nullptr) {
            OnTimerError("CreateTimer AllocObj failed.");
            return false;
        }

        qwDueTime /= qwTickOneSlotMS_;
        qwPeriod /= qwTickOneSlotMS_;

        pTimer->qwPeriod = qwPeriod;
        pTimer->timerFn = timerFn;
        pTimer->pParam = pParam;
        pTimer->qwTimerId = timerId;

        pTimer->bRunning = true;

        pTimer->qwExpires = this->qwCurrentTimeMS_ + qwDueTime;
        AddTimer(pTimer);
        this->mapTimers_[timerId] = pTimer;

        return true;
    }

    bool KillTimer(TimerIdType timerId) {
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

    void KillAllTimers() {
        for (auto it = this->mapTimers_.begin(); it != this->mapTimers_.end(); ++it) {
            it->second->bRunning = false;
        }
        this->mapTimers_.clear();
    }

   public:
    TimerMsType qwCurrentTimeMS_;  // current time ms
    const TimerMsType qwTickOneSlotMS_;
    std::unordered_map<TimerIdType, TimerNode*> mapTimers_;
    TimerNode* arrListTimerHead_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    TimerNode* arrListTimerTail_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];

   public:
    void Run() {
        TimerMsType idxExecutingSlotIdx = 0, idxNextWheelSlotIdx = 0;

        auto currTimeMS = UTimerGetCurrentTimeMS() / qwTickOneSlotMS_;
        TimerIdType timerId = 0;
        TimerNode* pTimer = nullptr;
        TimerNode* pNext = nullptr;
        while (currTimeMS >= this->qwCurrentTimeMS_) {
            idxExecutingSlotIdx = this->qwCurrentTimeMS_ & TIMER_MASK;

            idxNextWheelSlotIdx = idxExecutingSlotIdx;
            for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT - 1 && idxNextWheelSlotIdx == 0; ++i) {
                idxNextWheelSlotIdx = (this->qwCurrentTimeMS_ >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
                CascadeTimer(i + 1, idxNextWheelSlotIdx);
            }

            pTimer = this->arrListTimerHead_[0][idxExecutingSlotIdx];
            for (; pTimer != nullptr; pTimer = pNext) {
                pNext = pTimer->pNext;

                timerId = pTimer->qwTimerId;
                if (pTimer->bRunning) {
                    pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam);
                    if (pTimer->qwPeriod != 0) {
                        pTimer->qwExpires = this->qwCurrentTimeMS_ + pTimer->qwPeriod;
                        AddTimer(pTimer);
                    } else {
                        allocator_.FreeObj(pTimer);
                        this->mapTimers_.erase(timerId);
                    }
                } else {
                    allocator_.FreeObj(pTimer);
                }
            }
            this->arrListTimerHead_[0][idxExecutingSlotIdx] = nullptr;
            this->arrListTimerTail_[0][idxExecutingSlotIdx] = nullptr;

            ++this->qwCurrentTimeMS_;
        }
    }
};
