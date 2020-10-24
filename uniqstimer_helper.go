package uniqstimer

import (
	"fmt"
	"time"
)

type TimerIdType int64
type OnTimerFP func(TimerIdType, interface{})

var __pFreeTimerHeadMem *_TimerNode = nil
var UniqsTimerFreeCount int = 0

const UNIQS_TIMER_CACHE_MAX int = 4096
const UNIQS_TIMER_CACHE_DELETE int = UNIQS_TIMER_CACHE_MAX / 2

var __pFreeTimerHeadMemIII *_TimerNodeIII = nil
var UniqsTimerFreeCountIII int = 0

const UNIQS_TIMER_CACHE_MAXIII int = 4096
const UNIQS_TIMER_CACHE_DELETEIII int = UNIQS_TIMER_CACHE_MAXIII / 2

func AllocObj() *_TimerNode {
	if __pFreeTimerHeadMem != nil {
		UniqsTimerFreeCount--
		pTimer := __pFreeTimerHeadMem
		__pFreeTimerHeadMem = pTimer.pNext
		pTimer.pNext = nil
		return pTimer
	}
	ret := &_TimerNode{}
	return ret
}

func FreeObj(pTimer *_TimerNode) {
	UniqsTimerFreeCount++
	if __pFreeTimerHeadMem == nil {
		__pFreeTimerHeadMem = pTimer
		__pFreeTimerHeadMem.pNext = nil
	} else {
		pTimer.pNext = __pFreeTimerHeadMem
		__pFreeTimerHeadMem = pTimer
	}

	if UniqsTimerFreeCount > UNIQS_TIMER_CACHE_MAX {
		pDelete := __pFreeTimerHeadMem
		for i := 0; i < UNIQS_TIMER_CACHE_DELETE; i++ {
			__pFreeTimerHeadMem = pDelete.pNext

			// free memory
			//delete pDelete;

			pDelete = __pFreeTimerHeadMem
		}
		UniqsTimerFreeCount -= UNIQS_TIMER_CACHE_DELETE
	}
}

func AllocObjIII() *_TimerNodeIII {
	if __pFreeTimerHeadMemIII != nil {
		UniqsTimerFreeCountIII--
		pTimer := __pFreeTimerHeadMemIII
		__pFreeTimerHeadMemIII = pTimer.pNext
		pTimer.pNext = nil
		return pTimer
	}
	ret := &_TimerNodeIII{}
	return ret
}

func FreeObjIII(pTimer *_TimerNodeIII) {
	UniqsTimerFreeCount++
	if __pFreeTimerHeadMemIII == nil {
		__pFreeTimerHeadMemIII = pTimer
		__pFreeTimerHeadMemIII.pNext = nil
	} else {
		pTimer.pNext = __pFreeTimerHeadMemIII
		__pFreeTimerHeadMemIII = pTimer
	}

	if UniqsTimerFreeCountIII > UNIQS_TIMER_CACHE_MAXIII {
		pDelete := __pFreeTimerHeadMemIII
		for i := 0; i < UNIQS_TIMER_CACHE_DELETEIII; i++ {
			__pFreeTimerHeadMemIII = pDelete.pNext

			// free memory
			//delete pDelete;

			pDelete = __pFreeTimerHeadMemIII
		}
		UniqsTimerFreeCountIII -= UNIQS_TIMER_CACHE_DELETEIII
	}
}

func UTimerGetCurrentTimeMS() int64 {
	return time.Now().UnixNano() / 1000000
}

func OnTimerError(str string) {
	var errorStr = "OnTimerError" + str
	fmt.Print(errorStr)
	panic(errorStr)
}
