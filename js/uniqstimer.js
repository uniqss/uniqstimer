const TIMER_BITS_PER_WHEEL = 8
const TIMER_SLOT_COUNT_PER_WHEEL = 1 << 8
const TIMER_WHEEL_COUNT = 5
const TIMER_MASK = (1 << TIMER_BITS_PER_WHEEL) - 1

function AllocObj() {
    return new TimerNode();
}
function FreeObj(pTimer) {
}

function AddTimer(pTimerManager, pTimer, fromWheelIdx, fromSlotIdx) {
    wheelIdx = 0;
    slotIdx = 0;

    qwExpires = pTimer.qwExpires;
    qwDueTime = qwExpires - pTimerManager.qwCurrentTimeMS;

    if (false) { }
    else if (qwDueTime < 1 << (TIMER_BITS_PER_WHEEL * (0 + 1))) {
        slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 0)) & TIMER_MASK;
        wheelIdx = 0;
    }
    else if (qwDueTime < 1 << (TIMER_BITS_PER_WHEEL * (1 + 1))) {
        slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 1)) & TIMER_MASK;
        wheelIdx = 1;
    }
    else if (qwDueTime < (1) << (TIMER_BITS_PER_WHEEL * (2 + 1))) {
        slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 2)) & TIMER_MASK;
        wheelIdx = 2;
    }
    else if (qwDueTime < (1) << (TIMER_BITS_PER_WHEEL * (3 + 1))) {
        slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 3)) & TIMER_MASK;
        wheelIdx = 3;
    }
    else if (qwDueTime < (1) << (TIMER_BITS_PER_WHEEL * (4 + 1))) {
        slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 4)) & TIMER_MASK;
        wheelIdx = 4;
    }
    else if (qwDueTime < 0) {
        wheelIdx = 0;
        slotIdx = pTimerManager.qwCurrentTimeMS & TIMER_MASK;
    }
    else {
        OnTimerError("AddTimer this should not happen");
        return;
    }

    if (pTimerManager.arrListTimer[wheelIdx][slotIdx] == undefined) {
        pTimerManager.arrListTimer[wheelIdx][slotIdx] = pTimer;
        pTimer.pNext = undefined;
    }
    else {
        pTimer.pNext = pTimerManager.arrListTimer[wheelIdx][slotIdx];
        pTimerManager.arrListTimer[wheelIdx][slotIdx] = pTimer;
    }
}

// 循环该slot里的所有节点，重新AddTimer到管理器中
function CascadeTimer(pTimerManager, wheelIdx, slotIdx)
{
    pTimer = pTimerManager.arrListTimer[wheelIdx][slotIdx];
    pNext = undefined;
    for (; pTimer != undefined; pTimer = pNext) {
        pNext = pTimer.pNext;

        if (pTimer.bRunning) {
            AddTimer(pTimerManager, pTimer, wheelIdx, slotIdx);
        }
        else {
            // timer还没被放到最终执行轮就已经被kill掉了，需要释放掉
            //pTimerManager->pTimers.erase(pTimer->qwTimerId);
            FreeObj(pTimer);
        }
    }
    pTimerManager.arrListTimer[wheelIdx][slotIdx] = undefined;
}

class TimerNode {
    pNext;
    qwTimerId;
    qwExpires;
    qwPeriod;
    timerFn;
    pParam;
    bRunning;
    constructor() {
        console.log("uniqs TimerNode:constructor");
        this.pNext = undefined;
        this.qwTimerId = 0;
        this.qwExpires = 0;
        this.qwPeriod = 0;
        this.timerFn = undefined;
        this.pParam = undefined;
        this.bRunning = false;
    }
};

class TimerManager {
    constructor() {
        console.log("uniqs TimerManager:constructor");
        this.qwCurrentTimeMS = 0;
        this.pTimers = new Map();
        this.arrListTimer = new Array();
        for (var wheelidx = 0; wheelidx < TIMER_WHEEL_COUNT; wheelidx++) {
            this.arrListTimer[wheelidx] = new Array();
            for (var slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++) {
                this.arrListTimer[wheelidx][slotIdx] = undefined;
            }
        }
        this.qwCurrentTimeMS = this.UTimerGetCurrentTimeMS();
    }
    UTimerGetCurrentTimeMS() {
        return Date.now();
    }
    OnTimerError(str) {
        console.log(`OnTimerError str:${str}`);
    }
    Run() {
        idxExecutingSlotIdx = 0, idxNextWheelSlotIdx = 0;

        currTimeMS = this.UTimerGetCurrentTimeMS();
        timerId = 0;
        while (currTimeMS >= this.qwCurrentTimeMS) {
            idxExecutingSlotIdx = this.qwCurrentTimeMS & TIMER_MASK;

            idxNextWheelSlotIdx = idxExecutingSlotIdx;
            for (i = 0; i < 4 && idxNextWheelSlotIdx == 0; i++) {
                idxNextWheelSlotIdx = (this.qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
                CascadeTimer(this, i + 1, idxNextWheelSlotIdx);
            }

            pTimer = this.arrListTimer[0][idxExecutingSlotIdx];
            pNext = undefined;
            for (; pTimer != undefined; pTimer = pNext) {
                pNext = pTimer.pNext;

                timerId = pTimer.qwTimerId;
                if (pTimer.bRunning) {
                    pTimer.timerFn(pTimer.qwTimerId, pTimer.pParam);
                    if (pTimer.qwPeriod != 0) {
                        pTimer.qwExpires = this.qwCurrentTimeMS + pTimer.qwPeriod;
                        AddTimer(this, pTimer, 0, idxExecutingSlotIdx);
                    }
                    else {
                        FreeObj(pTimer);
                        pTimers.erase(timerId);
                    }
                }
                else {
                    FreeObj(pTimer);
                }
            }
            this.arrListTimer[0][idxExecutingSlotIdx] = undefined;

            this.qwCurrentTimeMS++;
        }
    }
    CreateTimer(timerId, timerFn, pParam, qwDueTime, qwPeriod) {
        if (undefined == timerFn)
            return false;

        // 两者都为0,无意义
        if (qwDueTime == 0 && qwPeriod == 0) {
            return false;
        }

        // 如果创建重复性定时器，又设置触发时间为0,就直接把时间设置为重复时间
        if (qwDueTime == 0) {
            return false;
        }

        if (this.pTimers.has(timerId)) {
            return false;
        }

        pTimer = AllocObj();
        if (pTimer == undefined) {
            this.OnTimerError("CreateTimer AllocObj failed.");
            return false;
        }

        pTimer.qwPeriod = qwPeriod;
        pTimer.timerFn = timerFn;
        pTimer.pParam = pParam;
        pTimer.qwTimerId = timerId;

        pTimer.bRunning = true;

        pTimer.qwExpires = this.qwCurrentTimeMS + qwDueTime;
        AddTimer(this, pTimer, 0, 0);
        this.pTimers[timerId] = pTimer;

        return true;
    }
    KillTimer(timerId) {
        pTimer = this.pTimers.get(timerId);
        if (pTimer == undefined) {
            return false;
        }
        if (!pTimer.bRunning) {
            return false;
        }

        pTimer.bRunning = false;
        this.pTimers.delete(it);

        return true;
    }
};

TimerManager.instance = new TimerManager();
module.exports = TimerManager.instance;
