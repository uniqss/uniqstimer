#pragma once

#include <unordered_map>
#include <stdint.h>

#define TimerIdType int64_t
#define TimerMsType int64_t

#define TIMER_BITS_PER_WHEEL 10
#define TIMER_WHEEL_COUNT 4

#define TIMER_SLOT_COUNT_PER_WHEEL 1 << TIMER_BITS_PER_WHEEL
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

struct UTimerNode {
    UTimerNode* pNext_;
    TimerIdType qwTimerId_;
    TimerMsType qwExpires_;  //
    TimerMsType qwPeriod_;
    void (*timerFn)(TimerIdType, void*);
    void* pParam_;
    bool bRunning_;
};

#include "timer_helper.h"

template <class NodeAllocator = UTimerNodeAllocator>
class TimerManager {
    NodeAllocator allocator_;

    void AddTimer(UTimerNode* pTimer) {
        TimerMsType wheelIdx = 0;
        TimerMsType slotIdx = 0;

        TimerMsType qwExpires = pTimer->qwExpires_;
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
            UTimerOnError("AddTimer this should not happen");
            return;
        }

        if (arrListTimerTail_[wheelIdx][slotIdx] == nullptr) {
            arrListTimerHead_[wheelIdx][slotIdx] = pTimer;
            arrListTimerTail_[wheelIdx][slotIdx] = pTimer;
            pTimer->pNext_ = nullptr;
        } else {
            arrListTimerTail_[wheelIdx][slotIdx]->pNext_ = pTimer;
            pTimer->pNext_ = nullptr;
            arrListTimerTail_[wheelIdx][slotIdx] = pTimer;
        }
    }

    void CascadeTimer(TimerMsType wheelIdx, TimerMsType slotIdx) {
        UTimerNode* pTimer = arrListTimerHead_[wheelIdx][slotIdx];
        UTimerNode* pNext = nullptr;
        for (; pTimer != nullptr; pTimer = pNext) {
            pNext = pTimer->pNext_;

            if (pTimer->bRunning_) {
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
        UTimerNode* pTimer = nullptr;
        UTimerNode* pNext = nullptr;
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                pTimer = this->arrListTimerHead_[wheelIdx][slotIdx];
                for (; pTimer != nullptr; pTimer = pNext) {
                    pNext = pTimer->pNext_;

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

        UTimerNode* pTimer = allocator_.AllocObj();
        if (pTimer == nullptr) {
            UTimerOnError("CreateTimer AllocObj failed.");
            return false;
        }

        qwDueTime /= qwTickOneSlotMS_;
        qwPeriod /= qwTickOneSlotMS_;

        pTimer->qwPeriod_ = qwPeriod;
        pTimer->timerFn = timerFn;
        pTimer->pParam_ = pParam;
        pTimer->qwTimerId_ = timerId;

        pTimer->bRunning_ = true;

        pTimer->qwExpires_ = this->qwCurrentTimeMS_ + qwDueTime;
        AddTimer(pTimer);
        this->mapTimers_[timerId] = pTimer;

        return true;
    }

    bool KillTimer(TimerIdType timerId) {
        auto it = this->mapTimers_.find(timerId);
        if (it == this->mapTimers_.end()) {
            return false;
        }

        UTimerNode* pTimer = it->second;
        if (!pTimer->bRunning_) {
            return false;
        }

        pTimer->bRunning_ = false;
        this->mapTimers_.erase(it);

        return true;
    }

    void KillAllTimers() {
        for (auto it = this->mapTimers_.begin(); it != this->mapTimers_.end(); ++it) {
            it->second->bRunning_ = false;
        }
        this->mapTimers_.clear();
    }

   public:
    TimerMsType qwCurrentTimeMS_;  // current time ms
    const TimerMsType qwTickOneSlotMS_;
    std::unordered_map<TimerIdType, UTimerNode*> mapTimers_;
    UTimerNode* arrListTimerHead_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    UTimerNode* arrListTimerTail_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];

   public:
    void Run() {
        TimerMsType idxExecutingSlotIdx = 0, idxNextWheelSlotIdx = 0;

        auto currTimeMS = UTimerGetCurrentTimeMS() / qwTickOneSlotMS_;
        TimerIdType timerId = 0;
        UTimerNode* pTimer = nullptr;
        UTimerNode* pNext = nullptr;
        while (currTimeMS >= this->qwCurrentTimeMS_) {
            idxExecutingSlotIdx = this->qwCurrentTimeMS_ & TIMER_MASK;

            idxNextWheelSlotIdx = idxExecutingSlotIdx;
            for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT - 1 && idxNextWheelSlotIdx == 0; ++i) {
                idxNextWheelSlotIdx = (this->qwCurrentTimeMS_ >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
                CascadeTimer(i + 1, idxNextWheelSlotIdx);
            }

            pTimer = this->arrListTimerHead_[0][idxExecutingSlotIdx];
            for (; pTimer != nullptr; pTimer = pNext) {
                pNext = pTimer->pNext_;

                timerId = pTimer->qwTimerId_;
                if (pTimer->bRunning_) {
                    pTimer->timerFn(pTimer->qwTimerId_, pTimer->pParam_);
                    if (pTimer->qwPeriod_ != 0) {
                        pTimer->qwExpires_ = this->qwCurrentTimeMS_ + pTimer->qwPeriod_;
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
