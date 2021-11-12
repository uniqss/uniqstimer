const TIMER_BITS_PER_WHEEL = 8;
const TIMER_SLOT_COUNT_PER_WHEEL = 1 << 8;
const TIMER_WHEEL_COUNT = 5;
const TIMER_MASK = (1 << TIMER_BITS_PER_WHEEL) - 1;

const ADDTIMER_SOURCE_NEW = 1;
const ADDTIMER_SOURCE_CASCADE = 2;
const ADDTIMER_SOURCE_ONTIMER = 3;

let __pFreeTimerHeadMem = undefined;
let UniqsTimerFreeCount = 0;
const UNIQS_TIMER_CACHE_MAX = 4096;
const UNIQS_TIMER_CACHE_DELETE = UNIQS_TIMER_CACHE_MAX / 2;

function AllocObj() {
    if (__pFreeTimerHeadMem !== undefined) {
        UniqsTimerFreeCount--;
        let pTimer = __pFreeTimerHeadMem;
        __pFreeTimerHeadMem = pTimer.pNext;
        pTimer.pNext = undefined;
        return pTimer;
    }
    return new TimerNode();
}
function FreeObj(pTimer) {
    UniqsTimerFreeCount++;
    if (__pFreeTimerHeadMem === undefined) {
        __pFreeTimerHeadMem = pTimer;
        __pFreeTimerHeadMem.pNext = undefined;
    }
    else {
        pTimer.pNext = __pFreeTimerHeadMem;
        __pFreeTimerHeadMem = pTimer;
    }

    if (UniqsTimerFreeCount > UNIQS_TIMER_CACHE_MAX) {
        let pDelete = __pFreeTimerHeadMem;
        for (let i = 0; i < UNIQS_TIMER_CACHE_DELETE; i++) {
            __pFreeTimerHeadMem = pDelete.pNext;

            // free memory
            // delete pDelete;

            pDelete = __pFreeTimerHeadMem;
        }
        UniqsTimerFreeCount -= UNIQS_TIMER_CACHE_DELETE;
    }
}

let AddTimer_EXCEED = Math.pow(2, TIMER_BITS_PER_WHEEL * TIMER_WHEEL_COUNT);
function AddTimer(pTimerManager, pTimer, fromWheelIdx, fromSlotIdx, source) {
    let wheelIdx = 0;
    let slotIdx = 0;

    let qwExpires = pTimer.qwExpires;
    let qwDueTime = qwExpires - pTimerManager.qwCurrentTimeMS;

    // console.log("AddTimer qwDueTime:", qwDueTime, " AddTimer_EXCEED:", AddTimer_EXCEED);
    if (qwDueTime < AddTimer_EXCEED) {
        for (let i = 0; i < TIMER_WHEEL_COUNT; i++) {
            if (qwDueTime < Math.pow(2, TIMER_BITS_PER_WHEEL * (i + 1))) {
                if (i === 0) {
                    slotIdx = qwExpires & TIMER_MASK;
                } else {
                    // slotIdx = Math.pow(qwExpires, 1 / (TIMER_BITS_PER_WHEEL * i)) & TIMER_MASK;
                    slotIdx = (qwExpires / Math.pow(2, TIMER_BITS_PER_WHEEL * i)) & TIMER_MASK;
                }
                wheelIdx = i;
                break;
            }
        }
    }
    else if (qwDueTime < 0) {
        wheelIdx = 0;
        slotIdx = pTimerManager.qwCurrentTimeMS & TIMER_MASK;
        // console.log(`AddTimer qwDueTime < 0 ${wheelIdx} ${slotIdx}`);
    }
    else {
        pTimerManager.OnTimerError(`AddTimer this should not happen qwExpires:${qwExpires} qwDueTime:${qwDueTime} ${TIMER_WHEEL_COUNT} ${TIMER_BITS_PER_WHEEL} ${EXCEED}`);
        return;
    }

    // console.log(`AddTimer ${wheelIdx} ${slotIdx} qwExpires:${qwExpires} qwDueTime:${qwDueTime} ${pTimerManager.qwCurrentTimeMS} ${source}`);
    if (pTimerManager.arrListTimerTail[wheelIdx][slotIdx] === undefined) {
        pTimerManager.arrListTimerHead[wheelIdx][slotIdx] = pTimer;
        pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = pTimer;
        pTimer.pNext = undefined;
    }
    else {
        pTimerManager.arrListTimerTail[wheelIdx][slotIdx].pNext = pTimer;
        pTimer.pNext = undefined;
        pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = pTimer;
    }
}

function CascadeTimer(pTimerManager, wheelIdx, slotIdx) {
    let pTimer = pTimerManager.arrListTimerHead[wheelIdx][slotIdx];
    let pNext = undefined;
    for (; pTimer !== undefined; pTimer = pNext) {
        pNext = pTimer.pNext;

        if (pTimer.bRunning) {
            // console.log(`CascadeTimer AddTimer ${wheelIdx} ${slotIdx}`);
            AddTimer(pTimerManager, pTimer, wheelIdx, slotIdx, ADDTIMER_SOURCE_CASCADE);
        } else {
            // console.log(`CascadeTimer FreeObj ${wheelIdx} ${slotIdx}`);
            FreeObj(pTimer);
        }
    }
    pTimerManager.arrListTimerHead[wheelIdx][slotIdx] = undefined;
    pTimerManager.arrListTimerTail[wheelIdx][slotIdx] = undefined;
}

class TimerNode {
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
        this.pTimers = new Map();
        this.arrListTimerHead = new Array();
        this.arrListTimerTail = new Array();
        for (let wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++) {
            this.arrListTimerHead[wheelIdx] = new Array();
            this.arrListTimerTail[wheelIdx] = new Array();
            for (let slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++) {
                this.arrListTimerHead[wheelIdx][slotIdx] = undefined;
                this.arrListTimerTail[wheelIdx][slotIdx] = undefined;
            }
        }
        this.qwCurrentTimeMS = this.UTimerGetCurrentTimeMS();
        console.log("uniqs TimerManager:constructor this.qwCurrentTimeMS:", this.qwCurrentTimeMS);
    }
    UTimerGetCurrentTimeMS() {
        return Date.now();
    }
    OnTimerError(str) {
        console.log(`OnTimerError str:${str}`);
    }
    Run() {
        let idxExecutingSlotIdx = 0; let idxNextWheelSlotIdx = 0;

        let currTimeMS = this.UTimerGetCurrentTimeMS();
        let timerId = 0;
        while (currTimeMS >= this.qwCurrentTimeMS) {
            idxExecutingSlotIdx = this.qwCurrentTimeMS & TIMER_MASK;
            // console.log(`Run idxExecutingSlotIdx:${idxExecutingSlotIdx}`);

            idxNextWheelSlotIdx = idxExecutingSlotIdx;
            for (let i = 0; i < TIMER_WHEEL_COUNT - 1 && idxNextWheelSlotIdx === 0; i++) {
                // idxNextWheelSlotIdx = Math.pow(this.qwCurrentTimeMS, 1 / ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
                idxNextWheelSlotIdx = this.qwCurrentTimeMS / Math.pow(2, (i + 1) * TIMER_BITS_PER_WHEEL) & TIMER_MASK;
                // console.log(`Run CascadeTimer ${i + 1} ${idxNextWheelSlotIdx} ${this.qwCurrentTimeMS}`);
                CascadeTimer(this, i + 1, idxNextWheelSlotIdx);
            }

            let pTimer = this.arrListTimerHead[0][idxExecutingSlotIdx];
            let pNext = undefined;

            // if (pTimer != undefined) console.log(`Run  executing 0 ${idxExecutingSlotIdx}`);

            for (; pTimer !== undefined; pTimer = pNext) {
                pNext = pTimer.pNext;

                timerId = pTimer.qwTimerId;
                if (pTimer.bRunning) {
                    pTimer.timerFn(pTimer.qwTimerId, pTimer.pParam);
                    if (pTimer.qwPeriod !== 0) {
                        pTimer.qwExpires = this.qwCurrentTimeMS + pTimer.qwPeriod;
                        AddTimer(this, pTimer, 0, idxExecutingSlotIdx, ADDTIMER_SOURCE_ONTIMER);
                    } else {
                        FreeObj(pTimer);
                        this.pTimers.delete(timerId);
                    }
                } else {
                    FreeObj(pTimer);
                }
            }
            this.arrListTimerHead[0][idxExecutingSlotIdx] = undefined;
            this.arrListTimerTail[0][idxExecutingSlotIdx] = undefined;

            this.qwCurrentTimeMS++;
        }
    }
    CreateTimer(timerId, timerFn, pParam, qwDueTime, qwPeriod) {
        if (undefined === timerFn)
            return false;

        // 两者都为0,无意义
        if (qwDueTime === 0 && qwPeriod === 0) {
            return false;
        }

        // 如果创建重复性定时器，又设置触发时间为0,就直接把时间设置为重复时间
        if (qwDueTime === 0) {
            return false;
        }

        if (this.pTimers.has(timerId)) {
            return false;
        }

        let pTimer = AllocObj();
        if (pTimer === undefined) {
            this.OnTimerError("CreateTimer AllocObj failed.");
            return false;
        }

        pTimer.qwPeriod = qwPeriod;
        pTimer.timerFn = timerFn;
        pTimer.pParam = pParam;
        pTimer.qwTimerId = timerId;

        pTimer.bRunning = true;

        pTimer.qwExpires = this.qwCurrentTimeMS + qwDueTime;
        // console.log(`CreateTimer ${timerId} qwDueTime:${qwDueTime} qwPeriod:${qwPeriod} this.qwCurrentTimeMS:${this.qwCurrentTimeMS} pTimer.qwExpires:${pTimer.qwExpires}`);
        AddTimer(this, pTimer, 0, 0, ADDTIMER_SOURCE_NEW);
        this.pTimers.set(timerId, pTimer);

        return true;
    }
    KillTimer(timerId) {
        let pTimer = this.pTimers.get(timerId);
        if (pTimer === undefined) {
            return false;
        }
        if (!pTimer.bRunning) {
            return false;
        }

        pTimer.bRunning = false;
        this.pTimers.delete(timerId);

        return true;
    }
};

TimerManager.instance = new TimerManager();
// export default TimerManager.instance;

module.exports = TimerManager.instance;
