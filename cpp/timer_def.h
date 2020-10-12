#pragma once

#define TimerMsType uint64_t
#define TimerIdType uint64_t

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define TIME_AFTER(a,b) ((TimerMsType)(b) - (TimerMsType)(a) < 0)
#define TIME_BEFORE(a,b) TIME_AFTER(b,a)
#define TIME_AFTER_EQ(a,b) ((TimerMsType)(a) >= (TimerMsType)(b))
#define TIME_BEFORE_EQ(a,b) TIME_AFTER_EQ(b,a)
