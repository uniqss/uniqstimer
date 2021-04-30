package uniqstimer

type TimerMsTypeIII int64

const TIMER_BITS_PER_WHEELIII = 6
const TIMER_SLOT_COUNT_PER_WHEELIII = 1 << TIMER_BITS_PER_WHEELIII
const TIMER_WHEEL_COUNTIII = 5
const TIMER_MASKIII = (1 << TIMER_BITS_PER_WHEELIII) - 1
const TIMER_MS_COUNTIII = 100

type _TimerNodeIII struct {
	pNext     *_TimerNodeIII
	qwTimerId TimerIdType
	qwExpires TimerMsTypeIII
	qwPeriod  TimerMsTypeIII
	timerFn   OnTimerFP
	pParam    interface{}
	bRunning  bool
}

type TimerManagerIII struct {
	qwCurrentTimeMS  TimerMsTypeIII
	pTimers          map[TimerIdType]*_TimerNodeIII
	arrListTimerHead [TIMER_WHEEL_COUNTIII][TIMER_SLOT_COUNT_PER_WHEELIII]*_TimerNodeIII
	arrListTimerTail [TIMER_WHEEL_COUNTIII][TIMER_SLOT_COUNT_PER_WHEELIII]*_TimerNodeIII
}

func (this *TimerManagerIII) Run() {
	var idxExecutingSlotIdx TimerMsTypeIII = 0
	var idxNextWheelSlotIdx TimerMsTypeIII = 0

	currTimeMS := TimerMsTypeIII(UTimerGetCurrentTimeMS() / TIMER_MS_COUNTIII)
	var timerId TimerIdType = 0
	for currTimeMS >= this.qwCurrentTimeMS {
		idxExecutingSlotIdx = this.qwCurrentTimeMS & TIMER_MASKIII

		idxNextWheelSlotIdx = idxExecutingSlotIdx
		var i TimerMsTypeIII = 0
		for ; i < TIMER_WHEEL_COUNTIII - 1 && idxNextWheelSlotIdx == 0; i++ {
			idxNextWheelSlotIdx = (this.qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEELIII)) & TIMER_MASKIII
			cascadeTimerIII(this, i+1, idxNextWheelSlotIdx)
		}

		pTimer := this.arrListTimerHead[0][idxExecutingSlotIdx]
		var pNext *_TimerNodeIII = nil
		for ; pTimer != nil; pTimer = pNext {
			pNext = pTimer.pNext

			timerId = pTimer.qwTimerId
			if pTimer.bRunning {
				pTimer.timerFn(pTimer.qwTimerId, pTimer.pParam)
				if pTimer.qwPeriod != 0 {
					pTimer.qwExpires = this.qwCurrentTimeMS + pTimer.qwPeriod
					addTimerIII(this, pTimer, 0, idxExecutingSlotIdx)
				} else {
					FreeObjIII(pTimer)
					delete(this.pTimers, timerId)
				}
			} else {
				FreeObjIII(pTimer)
			}
		}
		this.arrListTimerHead[0][idxExecutingSlotIdx] = nil
		this.arrListTimerTail[0][idxExecutingSlotIdx] = nil

		this.qwCurrentTimeMS++
	}
}

func NewTimerManagerIII() *TimerManagerIII {
	ret := &TimerManagerIII{}
	ret.pTimers = make(map[TimerIdType]*_TimerNodeIII)

	ret.qwCurrentTimeMS = TimerMsTypeIII(UTimerGetCurrentTimeMS() / TIMER_MS_COUNTIII)
	return ret
}
func DeleteTimerManagerIII(pTimerManager *TimerManagerIII) {
	if pTimerManager == nil {
		return
	}
	for wheelIdx := 0; wheelIdx < TIMER_WHEEL_COUNTIII; wheelIdx++ {
		for slotIdx := 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEELIII; slotIdx++ {
			pTimer := pTimerManager.arrListTimerHead[wheelIdx][slotIdx]
			var pNext *_TimerNodeIII = nil
			for ; pTimer != nil; pTimer = pNext {
				pNext = pTimer.pNext

				FreeObjIII(pTimer)
			}
		}
	}
	// delete pTimerManager;
}

// qwDueTime: first timeout   qwPeriod: then periodic timeout.(0: one shot timer)
func (this *TimerManagerIII) CreateTimer(timerId TimerIdType, timerFn OnTimerFP, pParam interface{}, qwDueTime TimerMsTypeIII, qwPeriod TimerMsTypeIII) bool {
	if timerFn == nil {
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
	pTimer := this.pTimers[timerId]
	if pTimer != nil {
		return false
	}

	pTimer = AllocObjIII()
	if pTimer == nil {
		OnTimerError("CreateTimerIII AllocObjIII failed.")
		return false
	}

	qwDueTime /= TIMER_MS_COUNTIII
	qwPeriod /= TIMER_MS_COUNTIII

	pTimer.qwPeriod = qwPeriod
	pTimer.timerFn = timerFn
	pTimer.pParam = pParam
	pTimer.qwTimerId = timerId

	pTimer.bRunning = true

	pTimer.qwExpires = this.qwCurrentTimeMS + qwDueTime
	addTimerIII(this, pTimer, 0, 0)
	this.pTimers[timerId] = pTimer

	return true
}

func (this *TimerManagerIII) KillTimer(timerId TimerIdType) bool {
	pTimer := this.pTimers[timerId]
	if pTimer == nil {
		return false
	}

	if !pTimer.bRunning {
		return false
	}

	pTimer.bRunning = false
	delete(this.pTimers, timerId)

	return true
}

func (this *TimerManagerIII) KillAllTimers() {
	for _, pTimer := range this.pTimers {
		pTimer.bRunning = false
	}

	for k := range this.pTimers {
		delete(this.pTimers, k)
	}
}

// inner functions

func addTimerIII(pTimerManager *TimerManagerIII, pTimer *_TimerNodeIII, fromWheelIdx TimerMsTypeIII, fromSlotIdx TimerMsTypeIII) {
	var wheelIdx TimerMsTypeIII = 0
	var slotIdx TimerMsTypeIII = 0

	qwExpires := pTimer.qwExpires
	qwDueTime := qwExpires - pTimerManager.qwCurrentTimeMS

	if qwDueTime < TimerMsTypeIII(1)<<(TIMER_BITS_PER_WHEELIII*(TIMER_WHEEL_COUNTIII)) {
		var i TimerMsTypeIII = 0
		for ; i < TIMER_WHEEL_COUNTIII; i++ {
			if qwDueTime < TimerMsTypeIII(1)<<(TIMER_BITS_PER_WHEELIII*(i+1)) {
				slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEELIII * i)) & TIMER_MASKIII
				wheelIdx = i
				break
			}
		}
	} else if qwDueTime < 0 {
		wheelIdx = 0
		slotIdx = pTimerManager.qwCurrentTimeMS & TIMER_MASKIII
	} else {
		OnTimerError("AddTimerIII this should not happen")
		return
	}

	if pTimerManager.arrListTimerTail[wheelIdx][slotIdx] == nil {
		pTimerManager.arrListTimerHead[wheelIdx][slotIdx] = pTimer
		pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = pTimer
		pTimer.pNext = nil
	} else {
		pTimerManager.arrListTimerTail[wheelIdx][slotIdx].pNext = pTimer
		pTimer.pNext = nil
		pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = pTimer
	}
}

func cascadeTimerIII(pTimerManager *TimerManagerIII, wheelIdx TimerMsTypeIII, slotIdx TimerMsTypeIII) {
	pTimer := pTimerManager.arrListTimerHead[wheelIdx][slotIdx]
	var pNext *_TimerNodeIII = nil
	for ; pTimer != nil; pTimer = pNext {
		pNext = pTimer.pNext

		if pTimer.bRunning {
			addTimerIII(pTimerManager, pTimer, wheelIdx, slotIdx)
		} else {
			FreeObjIII(pTimer)
		}
	}
	pTimerManager.arrListTimerHead[wheelIdx][slotIdx] = nil
	pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = nil
}
