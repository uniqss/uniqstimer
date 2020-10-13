#pragma once

#include<stdint.h>
#include<map>

// 100ms(<256) 5000ms(>256 && < 16384) 20000ms(>16384)
#define TestMSRound1 100
#define TestMSRound2 5000
#define TestMSRound3 20000

class TestCase {
public:
	uint64_t timerId;
	const char* msg;
	uint64_t dueTime;
	uint64_t period;
	int kill; // 如果<0跳 减，后判定是否=0，如=0,kill，如果bCreate为true,kill后create
	bool bCreate;
	uint64_t dueTimeNew;
	uint64_t periodNew;
	int triggerCount;
	bool isNew;
	int triggerCountNew;
public:
	TestCase()
	{

	}
	TestCase(uint64_t timerId, const char* msg, uint64_t dueTime, uint64_t period, int kill, bool bCreate, uint64_t dueTimeNew, uint64_t periodNew)
	{
		this->timerId = timerId;
		this->msg = msg;
		this->dueTime = dueTime;
		this->period = period;
		this->kill = kill;
		this->bCreate = bCreate;
		this->dueTimeNew = dueTimeNew;
		this->periodNew = periodNew;
		this->triggerCount = 0;
		this->isNew = false;
		this->triggerCountNew = 0;
	}
};

void init_testcase();
extern std::map<uint64_t, TestCase> g_TestCases;

// 一次性定时器，分多个轮
#define TestTimerIdOnce1 1
#define TestTimerIdOnce2 2
#define TestTimerIdOnce3 3
// 重复性定时器，首次分多个轮。
#define TestTimerIdRepeateFirst1 4
#define TestTimerIdRepeateFirst2 5
#define TestTimerIdRepeateFirst3 6
// 重复性定时器，后续分多个轮。
#define TestTimerIdRepeateRepeate1 7
#define TestTimerIdRepeateRepeate2 8
#define TestTimerIdRepeateRepeate3 9
// 一次性定时器，分多个轮，回调中kill掉自己
#define TestTimerIdOnceKill1 10
#define TestTimerIdOnceKill2 11
#define TestTimerIdOnceKill3 12
// 重复性定时器，首次分多个轮，回调中kill掉自己
#define TestTimerIdRepeateKillFirst1 13
#define TestTimerIdRepeateKillFirst2 14
#define TestTimerIdRepeateKillFirst3 15
// 重复性定时器，后续分多个轮，回调中kill掉自己
#define TestTimerIdRepeateKillRepeate1 16
#define TestTimerIdRepeateKillRepeate2 17
#define TestTimerIdRepeateKillRepeate3 18
// 一次性定时器，分多个轮，回调中kill掉自己，再启动一次性定时器，分多个轮
#define TestTimerIdOnceKillCreate11 2011
#define TestTimerIdOnceKillCreate12 2012
#define TestTimerIdOnceKillCreate13 2013
#define TestTimerIdOnceKillCreate21 2021
#define TestTimerIdOnceKillCreate22 2022
#define TestTimerIdOnceKillCreate23 2023
#define TestTimerIdOnceKillCreate31 2031
#define TestTimerIdOnceKillCreate32 2032
#define TestTimerIdOnceKillCreate33 2033
// 一次性定时器，分多个轮，回调中kill掉自己，再启动重复性定时器，首次分多个轮
#define TestTimerIdOnceKillCreateRepeateFirst11 3011
#define TestTimerIdOnceKillCreateRepeateFirst12 3012
#define TestTimerIdOnceKillCreateRepeateFirst13 3013
#define TestTimerIdOnceKillCreateRepeateFirst21 3021
#define TestTimerIdOnceKillCreateRepeateFirst22 3022
#define TestTimerIdOnceKillCreateRepeateFirst23 3023
#define TestTimerIdOnceKillCreateRepeateFirst31 3031
#define TestTimerIdOnceKillCreateRepeateFirst32 3032
#define TestTimerIdOnceKillCreateRepeateFirst33 3033
// 一次性定时器，分多个轮，回调中kill掉自己，再启动重复性定时器，后续分多个轮
#define TestTimerIdOnceKillCreateRepeateRepeate11 4011
#define TestTimerIdOnceKillCreateRepeateRepeate12 4012
#define TestTimerIdOnceKillCreateRepeateRepeate13 4013
#define TestTimerIdOnceKillCreateRepeateRepeate21 4021
#define TestTimerIdOnceKillCreateRepeateRepeate22 4022
#define TestTimerIdOnceKillCreateRepeateRepeate23 4023
#define TestTimerIdOnceKillCreateRepeateRepeate31 4031
#define TestTimerIdOnceKillCreateRepeateRepeate32 4032
#define TestTimerIdOnceKillCreateRepeateRepeate33 4033
// 重复性定时器，首次分多个轮，回调中kill掉自己，再启动一次性定时器，分多个轮
#define TestTimerIdRepeateFirstKillCreateOnce11 5011
#define TestTimerIdRepeateFirstKillCreateOnce12 5012
#define TestTimerIdRepeateFirstKillCreateOnce13 5013
#define TestTimerIdRepeateFirstKillCreateOnce21 5021
#define TestTimerIdRepeateFirstKillCreateOnce22 5022
#define TestTimerIdRepeateFirstKillCreateOnce23 5023
#define TestTimerIdRepeateFirstKillCreateOnce31 5031
#define TestTimerIdRepeateFirstKillCreateOnce32 5032
#define TestTimerIdRepeateFirstKillCreateOnce33 5033
// 重复性定时器，首次分多个轮，回调中kill掉自己，再启动重复性定时器，首次分多个轮
#define TestTimerIdRepeateFirstKillCreateRepeateFirst11 6011
#define TestTimerIdRepeateFirstKillCreateRepeateFirst12 6012
#define TestTimerIdRepeateFirstKillCreateRepeateFirst13 6013
#define TestTimerIdRepeateFirstKillCreateRepeateFirst21 6021
#define TestTimerIdRepeateFirstKillCreateRepeateFirst22 6022
#define TestTimerIdRepeateFirstKillCreateRepeateFirst23 6023
#define TestTimerIdRepeateFirstKillCreateRepeateFirst31 6031
#define TestTimerIdRepeateFirstKillCreateRepeateFirst32 6032
#define TestTimerIdRepeateFirstKillCreateRepeateFirst33 6033
// 重复性定时器，首次分多个轮，回调中kill掉自己，再启动重复性定时器，后续分多个轮
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate11 7011
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate12 7012
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate13 7013
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate21 7021
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate22 7022
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate23 7023
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate31 7031
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate32 7032
#define TestTimerIdRepeateFirstKillCreateRepeateRepeate33 7033
// 重复性定时器，后续分多个轮，回调中kill掉自己，再启动一次性定时器，分多个轮
#define TestTimerIdRepeateRepeateKillCreateOnce11 8011
#define TestTimerIdRepeateRepeateKillCreateOnce12 8012
#define TestTimerIdRepeateRepeateKillCreateOnce13 8013
#define TestTimerIdRepeateRepeateKillCreateOnce21 8021
#define TestTimerIdRepeateRepeateKillCreateOnce22 8022
#define TestTimerIdRepeateRepeateKillCreateOnce23 8023
#define TestTimerIdRepeateRepeateKillCreateOnce31 8031
#define TestTimerIdRepeateRepeateKillCreateOnce32 8032
#define TestTimerIdRepeateRepeateKillCreateOnce33 8033
// 重复性定时器，后续分多个轮，回调中kill掉自己，再启动重复性定时器，首次分多个轮
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce11 9011
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce12 9012
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce13 9013
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce21 9021
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce22 9022
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce23 9023
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce31 9031
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce32 9032
#define TestTimerIdRepeateRepeateKillCreateRepeateOnce33 9033
// 重复性定时器，后续分多个轮，回调中kill掉自己，再启动重复性定时器，后续分多个轮
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate11 10011
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate12 10012
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate13 10013
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate21 10021
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate22 10022
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate23 10023
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate31 10031
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate32 10032
#define TestTimerIdRepeateRepeateKillCreateRepeateRepeate33 10033
