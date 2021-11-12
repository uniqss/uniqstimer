let TimerManagerIII = require('./uniqstimerIII');
let updateInterval = 100;

if (1 === 1) {
} else {
    TimerManagerIII = require('./uniqstimer');
    updateInterval = 1;
}

const LoadAlarmFromDBSeconds = 1;
const WorkWheelCfgs = [
    { workWheelLevel: 1, reportIntervalSeconds: 5, maxReportCount: 5 },
    { workWheelLevel: 2, reportIntervalSeconds: 60, maxReportCount: 3 },
    { workWheelLevel: 3, reportIntervalSeconds: 300, maxReportCount: 0 },
];

function timeNow() {
    const date = new Date();
    let mm = pad(date.getMinutes(), 2)
    let ss = pad(date.getSeconds(), 2);
    let mmmm = pad(date.getMilliseconds(), 3);

    return mm + ss + mmmm;
}
function pad(number, length) {
    let str = '' + number;
    while (str.length < length) {
        str = '0' + str;
    }
    return str;
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function loadEventsFromDB() {
    console.log(`loadEventsFromDB now:${timeNow()}`);

    // // get last 100 data
    // const alarms = await Alarms.findAll({
    //     limits: 1000, order: ['id']
    // });
    //
    // alarms.forEach((alarm, idx) => {
    //     // const key = { topic: alarm.topic, alarmLevel: alarm.alarmLevel, desc: alarm.desc };
    // });
    // console.log(`alarms:${alarms} now:${timeNow()}`);
    await sleep(50);
    console.log(`loadEventsFromDB done now:${timeNow()}`);
}

async function tickWork(workWheelCfg) {
    console.log(`tickWork ${workWheelCfg.workWheelLevel} now:${timeNow()}`);
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function DoAnalysis() {
    if (1 === 0) {
        TimerManagerIII.CreateTimer(12345, async () => {
            await loadEventsFromDB();
        }, null, LoadAlarmFromDBSeconds * 1000, LoadAlarmFromDBSeconds * 1000);

        // await sleep(1000);

        WorkWheelCfgs.forEach((cfg) => {
            TimerManagerIII.CreateTimer(cfg.workWheelLevel, async () => {
                await tickWork(cfg);
            }, null, cfg.reportIntervalSeconds * 1000, cfg.reportIntervalSeconds * 1000);
        });
    }

    if (1 === 1) {
        let count = 0;
        const testDelCreateKey = 55555;
        TimerManagerIII.CreateTimer(testDelCreateKey, () => {
            console.log(`aaaaa testDelCreateKey:${testDelCreateKey}, count:${count}`);
            count++;
            if (count >= 3){
                const delok = TimerManagerIII.KillTimer(testDelCreateKey);
                console.log(`delok:${delok}`);
                const createOk = TimerManagerIII.CreateTimer(testDelCreateKey, ()=> {
                    console.log(`bbbbb testDelCreateKey:${testDelCreateKey}, count:${count}`);
                    count++;
                }, null, 1 * 1000, 1 * 1000)
                console.log(`createOk:${createOk}`);
            }
        }, null, 1 * 1000, 1 * 1000);
    }
}


DoAnalysis();

let lastTime = Date.now();
setInterval(() => {
    let currTime = Date.now();
    let diff = currTime - lastTime;
    while(diff > updateInterval){
        TimerManagerIII.Run();
        diff -= updateInterval;
    }
    lastTime = currTime - diff;
}, 1);

