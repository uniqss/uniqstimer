package uniqstimer

import (
	"fmt"
	"time"
)

type TimerMsType int64
type TimerIdType int64

const TIMER_BITS_PER_WHEEL = 8
const TIMER_SLOT_COUNT_PER_WHEEL = 1 << 8
const TIMER_WHEEL_COUNT = 5
const TIMER_MASK = (1 << TIMER_BITS_PER_WHEEL) - 1

type OnTimerFP func(TimerIdType, interface{})

type _TimerNode struct {
	pNext     *_TimerNode
	qwTimerId TimerIdType
	qwExpires TimerMsType
	qwPeriod  TimerMsType
	timerFn   OnTimerFP
	pParam    interface{}
	bRunning  bool
}

type TimerManager struct {
	qwCurrentTimeMS TimerMsType
	pTimers         map[TimerIdType]*_TimerNode
	arrListTimer    [TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL]*_TimerNode
}

func UTimerGetCurrentTimeMS() TimerMsType {
	return TimerMsType(time.Now().UnixNano() / 1000000)
}

func (this *TimerManager) Run() {
	var idxExecutingSlotIdx TimerMsType = 0
	var idxNextWheelSlotIdx TimerMsType = 0
	currTimeMS := UTimerGetCurrentTimeMS()
	var timerId TimerIdType = 0
	for ; currTimeMS >= this.qwCurrentTimeMS; {
		idxExecutingSlotIdx = this.qwCurrentTimeMS & TIMER_MASK

		idxNextWheelSlotIdx = idxExecutingSlotIdx
		var i TimerMsType = 0
		for ; i < 4 && idxNextWheelSlotIdx == 0; i++ {
			idxNextWheelSlotIdx = (this.qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK
			cascadeTimer(this, i+1, idxNextWheelSlotIdx)
		}

		pTimer := this.arrListTimer[0][idxExecutingSlotIdx]
		var pNext *_TimerNode = nil
		for ; pTimer != nil; pTimer = pNext {
			pNext = pTimer.pNext

			timerId = pTimer.qwTimerId
			if pTimer.bRunning {
				pTimer.timerFn(pTimer.qwTimerId, pTimer.pParam)
				if pTimer.qwPeriod != 0 {
					pTimer.qwExpires = this.qwCurrentTimeMS + pTimer.qwPeriod
					addTimer(this, pTimer, 0, idxExecutingSlotIdx)
				} else {
					FreeObj(pTimer)
					delete(this.pTimers, timerId)
				}
			} else {
				FreeObj(pTimer)
			}
		}
		this.arrListTimer[0][idxExecutingSlotIdx] = nil

		this.qwCurrentTimeMS++
	}
}

func CreateTimerManager() *TimerManager {
	ret := &TimerManager{}
	ret.pTimers = make(map[TimerIdType]*_TimerNode)
	ret.qwCurrentTimeMS = UTimerGetCurrentTimeMS()
	return ret
}
func DestroyTimerManager(pTimerManager *TimerManager) {
	if pTimerManager == nil {
		return
	}
	for wheelIdx := 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++ {
		for slotIdx := 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++ {
			pTimer := pTimerManager.arrListTimer[wheelIdx][slotIdx]
			var pNext *_TimerNode = nil
			for ; pTimer != nil; pTimer = pNext {
				pNext = pTimer.pNext

				FreeObj(pTimer)
			}
		}
	}
	// delete pTimerManager;
}

// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
func CreateTimer(pTimerManager *TimerManager, timerId TimerIdType, timerFn OnTimerFP, pParam interface{}, qwDueTime TimerMsType, qwPeriod TimerMsType) bool {
	if pTimerManager == nil || timerFn == nil {
		return false
	}

	// 两者都为0,无意义
	if qwDueTime == 0 && qwPeriod == 0 {
		return false
	}

	// 如果创建重复性定时器，又设置触发时间为0,就直接把时间设置为重复时间
	if qwDueTime == 0 {
		return false
	}
	pTimer := pTimerManager.pTimers[timerId]
	if pTimer != nil {
		return false
	}

	pTimer = AllocObj()
	if pTimer == nil {
		OnTimerError("CreateTimer AllocObj failed.")
		return false
	}

	pTimer.qwPeriod = qwPeriod
	pTimer.timerFn = timerFn
	pTimer.pParam = pParam
	pTimer.qwTimerId = timerId

	pTimer.bRunning = true

	pTimer.qwExpires = pTimerManager.qwCurrentTimeMS + qwDueTime
	addTimer(pTimerManager, pTimer, 0, 0)
	pTimerManager.pTimers[timerId] = pTimer

	return true
}

func KillTimer(pTimerManager *TimerManager, timerId TimerIdType) bool {
	if pTimerManager == nil {
		return false
	}
	pTimer := pTimerManager.pTimers[timerId]
	if pTimer == nil {
		return false
	}

	if !pTimer.bRunning {
		return false
	}

	pTimer.bRunning = false
	delete(pTimerManager.pTimers, timerId)

	return true
}

// inner functions

func OnTimerError(str string) {
	var errorStr = "OnTimerError" + str
	fmt.Print(errorStr)
	panic(errorStr)
}

func addTimer(pTimerManager *TimerManager, pTimer *_TimerNode, fromWheelIdx TimerMsType, fromSlotIdx TimerMsType) {
	var wheelIdx TimerMsType = 0
	var slotIdx TimerMsType = 0
	qwExpires := pTimer.qwExpires
	qwDueTime := qwExpires - pTimerManager.qwCurrentTimeMS

	if false {
	} else if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(0+1)) {
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 0)) & TIMER_MASK
		wheelIdx = 0
	} else if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(1+1)) {
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 1)) & TIMER_MASK
		wheelIdx = 1
	} else if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(2+1)) {
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 2)) & TIMER_MASK
		wheelIdx = 2
	} else if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(3+1)) {
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 3)) & TIMER_MASK
		wheelIdx = 3
	} else if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(4+1)) {
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 4)) & TIMER_MASK
		wheelIdx = 4
	} else if qwDueTime < 0 {
		wheelIdx = 0
		slotIdx = pTimerManager.qwCurrentTimeMS & TIMER_MASK
	} else {
		OnTimerError("addTimer this should not happen")
		return
	}

	if pTimerManager.arrListTimer[wheelIdx][slotIdx] == nil {
		pTimerManager.arrListTimer[wheelIdx][slotIdx] = pTimer
		pTimer.pNext = nil
	} else {
		pTimer.pNext = pTimerManager.arrListTimer[wheelIdx][slotIdx]
		pTimerManager.arrListTimer[wheelIdx][slotIdx] = pTimer
	}
}

func cascadeTimer(pTimerManager *TimerManager, wheelIdx TimerMsType, slotIdx TimerMsType) {
	pTimer := pTimerManager.arrListTimer[wheelIdx][slotIdx]
	var pNext *_TimerNode = nil
	for ; pTimer != nil; pTimer = pNext {
		pNext = pTimer.pNext

		if pTimer.bRunning {
			addTimer(pTimerManager, pTimer, wheelIdx, slotIdx)
		} else {
			FreeObj(pTimer)
		}
	}
	pTimerManager.arrListTimer[wheelIdx][slotIdx] = nil
}
