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

struct UTimerNode;
typedef void (*UTimerFn)(TimerIdType, UTimerNode*, void*);
struct UTimerNode {
    UTimerNode* next_;
    TimerIdType timerid_;
    TimerMsType expires_;  //
    TimerMsType period_;
    UTimerFn timerFn;
    void* param_;
    bool running_;
    void clear() { timerid_ = 0, expires_ = 0, period_ = 0, timerFn = nullptr, param_ = 0, running_ = 0; }
};

#include "timer_helper.h"

typedef TimerMsType (*TimerMgrGetMsFn)();
// not thread safe, designed to be used in one thread
template <class NodeAllocator = UTimerNodeAllocator>
class TimerManager {
   public:
    TimerManager(TimerMgrGetMsFn tmGetMsFn = UTimerGetCurrentTimeMS, TimerMsType tickOneSlotMS = 1)
        : tick_one_slot_ms_(tickOneSlotMS) {
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                list_timer_[wheelIdx][slotIdx].clear();
            }
        }

        get_curr_ms_fn_ = tmGetMsFn;
        run_time_ms_ = get_curr_ms_fn_() / tickOneSlotMS;
    }
    ~TimerManager() {
        UTimerNode* curr = nullptr;
        UTimerNode* next = nullptr;
        for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                curr = list_timer_[wheelIdx][slotIdx].head_;
                for (; curr != nullptr; curr = next) {
                    next = curr->next_;
                    curr->clear();
                    allocator_.FreeObj(curr);
                }
                list_timer_[wheelIdx][slotIdx].clear();
            }
        }
    }

    void Init(TimerMgrGetMsFn tmGetMsFn, TimerMsType tickOneSlotMS = 1) {
        get_curr_ms_fn_ = tmGetMsFn;
        tick_one_slot_ms_ = tickOneSlotMS;

        run_time_ms_ = get_curr_ms_fn_() / tickOneSlotMS;
    }

    // dueTime: first timeout. period: then periodic timeout.(0: one shot timer)    not thread safe.
    bool CreateTimer(TimerIdType timerId, UTimerFn timerFn, void* param, TimerMsType dueTime, TimerMsType period) {
        if (NULL == timerFn) return false;

        auto it = timers_.find(timerId);
        if (it != timers_.end()) {
            return false;
        }

        UTimerNode* curr = CreateTimerMem(timerFn, param, dueTime, period);
        if (curr == nullptr) {
            return false;
        }
        curr->timerid_ = timerId;
        timers_[timerId] = curr;

        return true;
    }

    // manage mem directly, no timerId. you MUST set the pointer you saved to null in your timerFn in timer's last
    // trigger
    UTimerNode* CreateTimerMem(UTimerFn timerFn, void* param, TimerMsType dueTime, TimerMsType period) {
        if (period < 0) period = 0;
        if (dueTime == 0) dueTime = tick_one_slot_ms_;

        UTimerNode* curr = allocator_.AllocObj();
        if (curr == nullptr) {
            UTimerOnError("CreateTimer AllocObj failed.");
            return nullptr;
        }

        if (dueTime > 0 && dueTime < tick_one_slot_ms_) dueTime = tick_one_slot_ms_;
        if (period > 0 && period < tick_one_slot_ms_) period = tick_one_slot_ms_;
        dueTime /= tick_one_slot_ms_;
        period /= tick_one_slot_ms_;

        curr->period_ = period;
        curr->timerFn = timerFn;
        curr->param_ = param;
        curr->timerid_ = 0;

        curr->running_ = true;

        curr->expires_ = run_time_ms_ + dueTime;
        AddTimer(curr);

        return curr;
    }

    // not thread safe.
    bool KillTimer(TimerIdType timerId) {
        auto it = timers_.find(timerId);
        if (it == timers_.end()) {
            return false;
        }

        UTimerNode* curr = it->second;
        if (!KillTimerMem(curr)) return false;

        timers_.erase(it);
        return true;
    }

    // manage mem directly, no timerId
    bool KillTimerMem(UTimerNode* timer) {
        if (!timer->running_) return false;
        timer->running_ = false;
        return true;
    }

    // not thread safe.
    void KillAllTimers() {
        for (auto it = timers_.begin(); it != timers_.end(); ++it) {
            it->second->running_ = false;
        }
        timers_.clear();

        for (int wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; ++wheelIdx) {
            for (int slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; ++slotIdx) {
                UTimerNode* curr = list_timer_[wheelIdx][slotIdx].head_;
                for (; curr != nullptr; curr = curr->next_) {
                    curr->running_ = false;
                }
            }
        }
    }

    // should be called in one thread, not thread safe
    void Run(int64_t currTimeMS = 0) {
        TimerMsType executingSlotIdx = 0, nextWheelSlotIdx = 0;

        if (currTimeMS == 0) currTimeMS = get_curr_ms_fn_() / tick_one_slot_ms_;
        TimerIdType timerId = 0;
        UTimerNode* curr = nullptr;
        UTimerNode* next = nullptr;
        while (currTimeMS >= run_time_ms_) {
            executingSlotIdx = run_time_ms_ & TIMER_MASK;

            nextWheelSlotIdx = executingSlotIdx;
            for (TimerMsType i = 0; i < TIMER_WHEEL_COUNT - 1 && nextWheelSlotIdx == 0; ++i) {
                nextWheelSlotIdx = (run_time_ms_ >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
                CascadeTimer(i + 1, nextWheelSlotIdx);
            }

            curr = list_timer_[0][executingSlotIdx].head_;
            while (curr != nullptr) {
                next = curr->next_;

                timerId = curr->timerid_;
                if (curr->running_) {
                    curr->timerFn(curr->timerid_, curr, curr->param_);
                    if (curr->period_ != 0) {
                        curr->expires_ = run_time_ms_ + curr->period_;
                        AddTimer(curr);
                    } else {
                        curr->clear();
                        allocator_.FreeObj(curr);
                        timers_.erase(timerId);
                    }
                } else {
                    curr->clear();
                    allocator_.FreeObj(curr);
                }

                curr = next;
            }
            list_timer_[0][executingSlotIdx].clear();

            ++run_time_ms_;
        }
    }

   private:
    void AddTimer(UTimerNode* curr) {
        TimerMsType wheelIdx = 0;
        TimerMsType slotIdx = 0;

        TimerMsType expires = curr->expires_;
        TimerMsType dueTime = expires - run_time_ms_;

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
            slotIdx = run_time_ms_ & TIMER_MASK;
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
                curr->clear();
                allocator_.FreeObj(curr);
            }
        }
        list_timer_[wheelIdx][slotIdx].clear();
    }

    TimerMgrGetMsFn get_curr_ms_fn_;
    TimerMsType run_time_ms_;  // current time ms
    TimerMsType tick_one_slot_ms_;

    // not thread safe.
    std::unordered_map<TimerIdType, UTimerNode*> timers_;

    UTimerPtrList<UTimerNode> list_timer_[TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL];
    NodeAllocator allocator_;
};
