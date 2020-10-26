package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"os"
	"time"
	"github.com/uniqss/uniqstimer"
)

var pMgr *uniqstimer.TimerManager = nil
var pMgrIII *uniqstimer.TimerManagerIII = nil

const timerIdMother uniqstimer.TimerIdType = 2
const timerIdMotherEnd uniqstimer.TimerIdType = 5000

var timerIdMotherCurr = timerIdMother

const timerIdRandStart uniqstimer.TimerIdType = 10000
const timerIdRandCount uniqstimer.TimerIdType = 6000000

var RunningTimersCount int = 100

var timerIdRandCurr uniqstimer.TimerIdType = timerIdRandStart

const TimerIdRandStop uniqstimer.TimerIdType = timerIdRandStart + timerIdRandCount

type TestRandTimerInfo struct {
	state                 int // 0默认 1created 2killed
	triggeredCount        int
	isRepeate             bool
	triggeredCountRepeate int
	createTime            uniqstimer.TimerMsType
	dueTime               uniqstimer.TimerMsType
	period                uniqstimer.TimerMsType
}

func (this *TestRandTimerInfo) Clear() {
	this.state = 0
	this.triggeredCount = 0
	this.isRepeate = false
	this.triggeredCountRepeate = 0
	this.createTime = 0
	this.dueTime = 0
	this.period = 0
}

func NewTestRandTimerInfo() *TestRandTimerInfo {
	return &TestRandTimerInfo{
	}
}

var arrTestRandTimerInfos [timerIdRandCount]*TestRandTimerInfo

var lastTimeMS = uniqstimer.UTimerGetCurrentTimeMS()
var OnTimerTriggered uniqstimer.TimerMsType= 0

func OnTimerIII(timerId uniqstimer.TimerIdType, pParam interface{}) {
	fmt.Println("OnTimerIII ", timerId)
}

func OnTimer(timerId uniqstimer.TimerIdType, pParam interface{}) {
	//fmt.Print("OnTimer ", timerId)
	currMS := uniqstimer.UTimerGetCurrentTimeMS()

	if timerId == 1 {
		if timerIdMotherCurr <= timerIdMotherEnd {
			for i := 1; i < 10 && timerIdMotherCurr <= timerIdMotherEnd; i++ {
				pMgr.CreateTimer(timerIdMotherCurr, OnTimer, "mother", 500, 500)
				timerIdMotherCurr++
			}
		} else {
			pMgr.KillTimer(timerId)
		}
	} else if timerId <= timerIdMotherEnd {
		// rand crate kill
		randTimerIdIdx := uniqstimer.TimerIdType(rand.Int63()) % timerIdRandCount
		randTimerId := timerIdRandStart + randTimerIdIdx
		rInfo := arrTestRandTimerInfos[randTimerIdIdx]

		// kill
		var __randKill = uniqstimer.TimerIdType(rand.Int() % 10000)
		var __randKillPercent uniqstimer.TimerIdType = 5000
		if __randKill < __randKillPercent {
			if rInfo.state == 1 {
				bOk := pMgr.KillTimer(randTimerId)
				if !bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
					//printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
				}
				// set data
				RunningTimersCount--
				rInfo.Clear()
				rInfo.state = 2
				bOk = pMgr.KillTimer(randTimerId)
				if bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
				}
			} else {
				bOk := pMgr.KillTimer(randTimerId)
				if bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
				}
				rInfo.state = 2
			}
		}

		// create
		var randCreate uniqstimer.TimerIdType = uniqstimer.TimerIdType(rand.Int() % 10000)
		if uniqstimer.TimerIdType(RunningTimersCount) < timerIdRandCount && randCreate < 8000 {
			var randRepeate bool = (rand.Int() % 2) > 0
			var t1 uniqstimer.TimerMsType = uniqstimer.TimerMsType((rand.Int() % (131072 + 6)) + 1)
			var t2 uniqstimer.TimerMsType = 0
			if randRepeate {
				t2 = uniqstimer.TimerMsType((rand.Int() % (131072 + 6)) + 1)
			}

			if rInfo.state == 1 {
				bOk := pMgr.CreateTimer(randTimerId, OnTimer, "", t1, t2)
				if bOk {
					uniqstimer.OnTimerError("OnTimer CreateTimer should fail but succeeded. randTimerId:")
				}
			} else {
				bOk := pMgr.CreateTimer(randTimerId, OnTimer, "", t1, t2)
				if !bOk {
					uniqstimer.OnTimerError("OnTimer CreateTimer should succeed but failed. randTimerId:")
				}
				// set data
				RunningTimersCount++
				rInfo.state = 1
				rInfo.triggeredCount = 0
				rInfo.isRepeate = randRepeate
				rInfo.createTime = uniqstimer.TimerMsType(currMS)
				rInfo.dueTime = t1
				rInfo.period = t2
				bOk = pMgr.CreateTimer(randTimerId, OnTimer, "", t1, t2)
				if bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
				}
			}
			pMgr.CreateTimer(randTimerId, OnTimer, "rand", t1, t2)
		}
	}

	diff := currMS - lastTimeMS
	OnTimerTriggered++
	if diff > 1000 {
		fmt.Printf("RunningTimersCount:%d OnTimerTriggered:%v\n", RunningTimersCount, OnTimerTriggered)
		lastTimeMS = currMS
	}

	if timerId >= timerIdRandStart {
		timerIdIdx := timerId - timerIdRandStart
		rInfo := arrTestRandTimerInfos[timerIdIdx]
		shouldTimeMS := rInfo.createTime
		if rInfo.dueTime == 0 {
			rInfo.triggeredCountRepeate++
		} else if rInfo.isRepeate && rInfo.triggeredCount > 0 {
			rInfo.triggeredCountRepeate++
		} else {
			rInfo.triggeredCount++
		}

		shouldTimeMS += rInfo.dueTime*uniqstimer.TimerMsType(rInfo.triggeredCount) + rInfo.period*uniqstimer.TimerMsType(rInfo.triggeredCountRepeate)

		var __diff uniqstimer.TimerMsType = 0
		currTimeMs := uniqstimer.TimerMsType(currMS)
		if shouldTimeMS > currTimeMs {
			__diff = shouldTimeMS - currTimeMs
		} else {
			__diff = currTimeMs - shouldTimeMS
		}
		if __diff > 100 {
			uniqstimer.OnTimerError("timer time check error")
		}

		if !rInfo.isRepeate {
			rInfo.state = 0
		}
	}
}

var working = true

func logicThread() {
	pMgr.CreateTimer(1, OnTimer, "root", 500, 500)

	pMgrIII.CreateTimer(1, OnTimerIII, "timer III", 500, 500)

	var lastMS int64 = 0
	for working {
		currMS := uniqstimer.UTimerGetCurrentTimeMS()
		if currMS - lastMS > 100 {
			pMgrIII.Run()
		}
		pMgr.Run()
		time.Sleep(time.Microsecond * 100)
	}
}

func main() {
	var i uniqstimer.TimerIdType = 0
	for ; i < timerIdRandCount; i++ {
		arrTestRandTimerInfos[i] = NewTestRandTimerInfo()
	}
	pMgr = uniqstimer.NewTimerManager()
	pMgrIII = uniqstimer.NewTimerManagerIII()
	go logicThread()

	for working {
		var reader = bufio.NewReader(os.Stdin)

		line, _, _ := reader.ReadLine()

		str := string(line)

		if str == "e" || str == "exit" {
			working = false
		}
	}

	uniqstimer.DeleteTimerManager(pMgr)
	uniqstimer.DeleteTimerManagerIII(pMgrIII)
}
