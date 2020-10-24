package uniqstimer

type TimerMsType int64

const TIMER_BITS_PER_WHEEL = 8
const TIMER_SLOT_COUNT_PER_WHEEL = 1 << TIMER_BITS_PER_WHEEL
const TIMER_WHEEL_COUNT = 5
const TIMER_MASK = (1 << TIMER_BITS_PER_WHEEL) - 1

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
	qwCurrentTimeMS  TimerMsType
	pTimers          map[TimerIdType]*_TimerNode
	arrListTimerHead [TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL]*_TimerNode
	arrListTimerTail [TIMER_WHEEL_COUNT][TIMER_SLOT_COUNT_PER_WHEEL]*_TimerNode
}

func (this *TimerManager) Run() {
	var idxExecutingSlotIdx TimerMsType = 0
	var idxNextWheelSlotIdx TimerMsType = 0

	currTimeMS := TimerMsType(UTimerGetCurrentTimeMS())
	var timerId TimerIdType = 0
	for currTimeMS >= this.qwCurrentTimeMS {
		idxExecutingSlotIdx = this.qwCurrentTimeMS & TIMER_MASK

		idxNextWheelSlotIdx = idxExecutingSlotIdx
		var i TimerMsType = 0
		for ; i < 4 && idxNextWheelSlotIdx == 0; i++ {
			idxNextWheelSlotIdx = (this.qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK
			cascadeTimer(this, i+1, idxNextWheelSlotIdx)
		}

		pTimer := this.arrListTimerHead[0][idxExecutingSlotIdx]
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
		this.arrListTimerHead[0][idxExecutingSlotIdx] = nil
		this.arrListTimerTail[0][idxExecutingSlotIdx] = nil

		this.qwCurrentTimeMS++
	}
}

func NewTimerManager() *TimerManager {
	ret := &TimerManager{}
	ret.pTimers = make(map[TimerIdType]*_TimerNode)

	ret.qwCurrentTimeMS = TimerMsType(UTimerGetCurrentTimeMS())
	return ret
}
func DeleteTimerManager(pTimerManager *TimerManager) {
	if pTimerManager == nil {
		return
	}
	for wheelIdx := 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++ {
		for slotIdx := 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++ {
			pTimer := pTimerManager.arrListTimerHead[wheelIdx][slotIdx]
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
func (this *TimerManager) CreateTimer(timerId TimerIdType, timerFn OnTimerFP, pParam interface{}, qwDueTime TimerMsType, qwPeriod TimerMsType) bool {
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

	pTimer.qwExpires = this.qwCurrentTimeMS + qwDueTime
	addTimer(this, pTimer, 0, 0)
	this.pTimers[timerId] = pTimer

	return true
}

func (this *TimerManager) KillTimer(timerId TimerIdType) bool {
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

// inner functions

func addTimer(pTimerManager *TimerManager, pTimer *_TimerNode, fromWheelIdx TimerMsType, fromSlotIdx TimerMsType) {
	var wheelIdx TimerMsType = 0
	var slotIdx TimerMsType = 0

	qwExpires := pTimer.qwExpires
	qwDueTime := qwExpires - pTimerManager.qwCurrentTimeMS

	if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(TIMER_WHEEL_COUNT)) {
		var i TimerMsType = 0
		for ; i < TIMER_WHEEL_COUNT; i++ {
			if qwDueTime < TimerMsType(1)<<(TIMER_BITS_PER_WHEEL*(i+1)) {
				slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * i)) & TIMER_MASK
				wheelIdx = i
				break
			}
		}
	} else if qwDueTime < 0 {
		wheelIdx = 0
		slotIdx = pTimerManager.qwCurrentTimeMS & TIMER_MASK
	} else {
		OnTimerError("AddTimer this should not happen")
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

func cascadeTimer(pTimerManager *TimerManager, wheelIdx TimerMsType, slotIdx TimerMsType) {
	pTimer := pTimerManager.arrListTimerHead[wheelIdx][slotIdx]
	var pNext *_TimerNode = nil
	for ; pTimer != nil; pTimer = pNext {
		pNext = pTimer.pNext

		if pTimer.bRunning {
			addTimer(pTimerManager, pTimer, wheelIdx, slotIdx)
		} else {
			FreeObj(pTimer)
		}
	}
	pTimerManager.arrListTimerHead[wheelIdx][slotIdx] = nil
	pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = nil
}
