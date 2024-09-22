#pragma once

#include <unordered_map>
#include <stdint.h>

#define TimerIdType int64_t
#define TimerMsType int64_t

#define TIMER_BITS_PER_WHEEL 10
#define TIMER_WHEEL_COUNT 4

#define TIMER_SLOT_COUNT_PER_WHEEL 1 << TIMER_BITS_PER_WHEEL
#define TIMER_MASK ((1 << TIMER_BITS_PER_WHEEL) - 1)

template <typename T>
struct UTimerPtrList {
    T* head_ = nullptr;
    T* tail_ = nullptr;
    inline void clear() { head_ = tail_ = nullptr; }
    inline void push_back(T* ptr) {
        if (head_ == nullptr) {
            head_ = tail_ = ptr;
        } else {
            tail_->next_ = ptr;
            tail_ = ptr;
        }
    }
};

struct UTimerNode {
    UTimerNode* next_;
    TimerIdType timerid_;
    TimerMsType expires_;  //
    TimerMsType period_;
    void (*timerFn)(TimerIdType, void*);
    void* param_;
    bool running_;
};

#include "timer_helper.h"

// not thread safe, designed to be used in one thread
template <class NodeAllocator = UTimerNodeAllocator>
class TimerManager {
   public:
    TimerManager(TimerMsType tickOneSlotMS = 1) : tick_one_slot_ms_(tickOneSlotMS) {
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                list_timer_[wheelIdx][slotIdx].clear();
            }
        }

        this->current_time_ms_ = UTimerGetCurrentTimeMS() / tick_one_slot_ms_;
    }
    ~TimerManager() {
        UTimerNode* curr = nullptr;
        UTimerNode* next = nullptr;
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                curr = list_timer_[wheelIdx][slotIdx].head_;
                for (; curr != nullptr; curr = next) {
                    next = curr->next_;

                    allocator_.FreeObj(curr);
                }
                list_timer_[wheelIdx][slotIdx].clear();
            }
        }
    }

    // dueTime: first timeout. period: then periodic timeout.(0: one shot timer)    not thread safe.
    bool CreateTimer(TimerIdType timerId, void (*timerFn)(TimerIdType, void*), void* param, TimerMsType dueTime, TimerMsType period) {
        if (NULL == timerFn) return false;

        // both 0, meaningless
        if (dueTime == 0 && period == 0) {
            return false;
        }

        // if repeat, and duetime = 0, set duetime to period
        if (dueTime == 0) {
            return false;
        }

        auto it = this->timers_.find(timerId);
        if (it != this->timers_.end()) {
            return false;
        }

        UTimerNode* curr = allocator_.AllocObj();
        if (curr == nullptr) {
            UTimerOnError("CreateTimer AllocObj failed.");
            return false;
        }

        dueTime /= tick_one_slot_ms_;
        period /= tick_one_slot_ms_;

        curr->period_ = period;
        curr->timerFn = timerFn;
        curr->param_ = param;
        curr->timerid_ = timerId;

        curr->running_ = true;

        curr->expires_ = this->current_time_ms_ + dueTime;
        AddTimer(curr);
        this->timers_[timerId] = curr;

        return true;
    }

    // not thread safe.
    bool KillTimer(TimerIdType timerId) {
        auto it = this->timers_.find(timerId);
        if (it == this->timers_.end()) {
            return false;
        }

        UTimerNode* curr = it->second;
        if (!curr->running_) {
            return false;
        }

        curr->running_ = false;
        this->timers_.erase(it);

        return true;
    }

    // not thread safe.
    void KillAllTimers() {
        for (auto it = this->timers_.begin(); it != this->timers_.end(); ++it) {
            it->second->running_ = false;
        }
        this->timers_.clear();
    }

    // should be called in one thread, not thread safe
    void Run() {
        TimerMsType executingSlotIdx = 0, nextWheelSlotIdx = 0;

        auto currTimeMS = UTimerGetCurrentTimeMS() / tick_one_slot_ms_;
        TimerIdType timerId = 0;
        UTimerNode* curr = nullptr;
        UTimerNode* next = nullptr;
        while (currTimeMS >= this->current_time_ms_) {
            executingSlotIdx = this->current_time_ms_ & TIMER_MASK;

            nextWheelSlotIdx = executingSlotIdx;
            for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT - 1 && nextWheelSlotIdx == 0; ++i) {
                nextWheelSlotIdx = (this->current_time_ms_ >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
                CascadeTimer(i + 1, nextWheelSlotIdx);
            }

            curr = list_timer_[0][executingSlotIdx].head_;
            while (curr != nullptr) {
                next = curr->next_;

                timerId = curr->timerid_;
                if (curr->running_) {
                    curr->timerFn(curr->timerid_, curr->param_);
                    if (curr->period_ != 0) {
                        curr->expires_ = this->current_time_ms_ + curr->period_;
                        AddTimer(curr);
                    } else {
                        allocator_.FreeObj(curr);
                        this->timers_.erase(timerId);
                    }
                } else {
                    allocator_.FreeObj(curr);
                }

                curr = next;
            }
            list_timer_[0][executingSlotIdx].clear();

            ++this->current_time_ms_;
        }
    }

   private:
    void AddTimer(UTimerNode* curr) {
        TimerMsType wheelIdx = 0;
        TimerMsType slotIdx = 0;

        TimerMsType expires = curr->expires_;
        TimerMsType dueTime = expires - this->current_time_ms_;

        if (dueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (TIMER_WHEEL_COUNT))) {
            for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT; ++i) {
                if (dueTime < TimerMsType(1) << (TIMER_BITS_PER_WHEEL * (i + 1))) {
                    slotIdx = (expires >> (TIMER_BITS_PER_WHEEL * i)) & TIMER_MASK;
                    wheelIdx = i;
                    break;
                }
            }
        } else if (dueTime < 0) {
            wheelIdx = 0;
            slotIdx = this->current_time_ms_ & TIMER_MASK;
        } else {
            UTimerOnError("AddTimer this should not happen");
            return;
        }

        list_timer_[wheelIdx][slotIdx].push_back(curr);
        curr->next_ = nullptr;
    }

    void CascadeTimer(TimerMsType wheelIdx, TimerMsType slotIdx) {
        UTimerNode* curr = list_timer_[wheelIdx][slotIdx].head_;
        UTimerNode* next = nullptr;
        for (; curr != nullptr; curr = next) {
            next = curr->next_;

            if (curr->running_) {
                AddTimer(curr);
            } else {
                allocator_.FreeObj(curr);
            }
        }
        list_timer_[wheelIdx][slotIdx].clear();
    }

    TimerMsType current_time_ms_;  // current time ms
    const TimerMsType tick_one_slot_ms_;

    // not thread safe.
    std::unordered_map<TimerIdType, UTimerNode*> timers_;

    UTimerNode* list_timer_head_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    UTimerNode* list_timer_tail_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    UTimerPtrList<UTimerNode> list_timer_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    NodeAllocator allocator_;
};
