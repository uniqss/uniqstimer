package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"os"
	"time"
	"uniqstimer"
)

var pMgr *uniqstimer.TimerManager = nil

const timerIdMother uniqstimer.TimerIdType = 2
const timerIdMotherEnd uniqstimer.TimerIdType = 5000

var timerIdMotherCurr = timerIdMother

const timerIdRandStart uniqstimer.TimerIdType = 10000
const timerIdRandCount uniqstimer.TimerIdType = 1000000

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

func OnTimer(timerId uniqstimer.TimerIdType, pParam interface{}) {
	//fmt.Print("OnTimer ", timerId)
	currMS := uniqstimer.UTimerGetCurrentTimeMS()

	if timerId == 1 {
		if timerIdMotherCurr <= timerIdMotherEnd {
			for i := 1; i < 10 && timerIdMotherCurr <= timerIdMotherEnd; i++ {
				uniqstimer.CreateTimer(pMgr, timerIdMotherCurr, OnTimer, "mother", 500, 500)
				timerIdMotherCurr++
			}
		} else {
			uniqstimer.KillTimer(pMgr, timerId)
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
				bOk := uniqstimer.KillTimer(pMgr, randTimerId)
				if !bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
					//printf("OnTimer KillTimer failed. timerId:%llu\n", timerId);
				}
				// set data
				RunningTimersCount--
				rInfo.Clear()
				rInfo.state = 2
				bOk = uniqstimer.KillTimer(pMgr, randTimerId)
				if bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
				}
			} else {
				bOk := uniqstimer.KillTimer(pMgr, randTimerId)
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
				bOk := uniqstimer.CreateTimer(pMgr, randTimerId, OnTimer, "", t1, t2)
				if bOk {
					uniqstimer.OnTimerError("OnTimer CreateTimer should fail but succeeded. randTimerId:")
				}
			} else {
				bOk := uniqstimer.CreateTimer(pMgr, randTimerId, OnTimer, "", t1, t2)
				if !bOk {
					uniqstimer.OnTimerError("OnTimer CreateTimer should succeed but failed. randTimerId:")
				}
				// set data
				RunningTimersCount++
				rInfo.state = 1
				rInfo.triggeredCount = 0
				rInfo.isRepeate = randRepeate
				rInfo.createTime = currMS
				rInfo.dueTime = t1
				rInfo.period = t2
				bOk = uniqstimer.CreateTimer(pMgr, randTimerId, OnTimer, "", t1, t2)
				if bOk {
					uniqstimer.OnTimerError("OnTimer KillTimer failed randTimerId:")
				}
			}
			uniqstimer.CreateTimer(pMgr, randTimerId, OnTimer, "rand", t1, t2)
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
		if shouldTimeMS > currMS {
			__diff = shouldTimeMS - currMS
		} else {
			__diff = currMS - shouldTimeMS
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
	uniqstimer.CreateTimer(pMgr, 1, OnTimer, "root", 500, 500)
	for working {
		pMgr.Run()
		time.Sleep(time.Microsecond * 500)
	}
}

func main() {
	var i uniqstimer.TimerIdType = 0
	for ; i < timerIdRandCount; i++ {
		arrTestRandTimerInfos[i] = NewTestRandTimerInfo()
	}
	pMgr = uniqstimer.CreateTimerManager()
	go logicThread()

	for working {
		var reader = bufio.NewReader(os.Stdin)

		line, _, _ := reader.ReadLine()

		str := string(line)

		if str == "e" || str == "exit" {
			working = false
		}
	}

	uniqstimer.DestroyTimerManager(pMgr)
}
