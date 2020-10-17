#include "Timer.h"

#include "timer_mem.h"

#include <stddef.h>
#include <stdlib.h>
#include<chrono>
#include <cassert>
#include<string>


#ifdef UNIQS_DEBUG_TIMER
TimerMsType DebugDiffTimeMs = 0;
#endif

enum EFucking
{
	EFucking_Crateing,
	EFucking_Killing1,
	EFucking_Killing2,
	EFucking_Cascading1,
	EFucking_Cascading2,
	EFucking_Run1,
	EFucking_Run2,
	EFucking_Run3,
	EFucking_Add1,
};

std::unordered_map<TimerNode*, std::list<std::tuple<EFucking, TimerIdType, TimerMsType, std::string>>> fucking;

TimerMsType UTimerGetCurrentTimeMS(void)
{
	auto time_now = std::chrono::system_clock::now();
	auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
#ifdef UNIQS_DEBUG_TIMER
	return (TimerMsType)duration_in_ms.count() - DebugDiffTimeMs;
#else
	return (TimerMsType)duration_in_ms.count();
#endif
}

void fuck(TimerNode* pTimer)
{
	auto it = fucking[pTimer];
	static int size = it.size();
}

void OnTimerError(const std::string& err)
{
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "OnTimer error err:" << err;
#endif
	throw std::logic_error("OnTimer error err:" + err);

	//printf("OnTimer error err:%s\n", err.c_str());
}

enum EAddTimerSource
{
	EAddTimerSource_Create_NewAlloc,
	EAddTimerSource_Cascade,
	EAddTimerSource_TimeoutReadd,
};
const char* pszAddTimerSource[] = {
	"EAddTimerSource_Create_NewAlloc",
	"EAddTimerSource_Cascade",
	"EAddTimerSource_TimeoutReadd",
};

static void AddTimer(TimerManager* pTimerManager, TimerNode* pTimer, TimerMsType fromWheelIdx, TimerMsType fromSlotIdx, EAddTimerSource source)
{
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "AddTimer timerId:"<<pTimer->qwTimerId<<"fromWheelIdx:" << fromWheelIdx << " fromSlotIdx:" << fromSlotIdx << " source:" << pszAddTimerSource[source];
#endif

	TimerMsType qwDueTime, qwExpires;

	qwExpires = pTimer->qwExpires;
	qwDueTime = qwExpires - pTimerManager->qwCurrentTimeMS;

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
	bool negative = false;
#endif

	int wheelIdx = 0;
	int slotIdx = 0;

	if (false) {}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (0 + 1)))
	{
		wheelIdx = 0;
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 0)) & TIMER_MASK;
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (1 + 1)))
	{
		wheelIdx = 1;
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 1)) & TIMER_MASK;
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (2 + 1)))
	{
		wheelIdx = 2;
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 2)) & TIMER_MASK;
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (3 + 1)))
	{
		wheelIdx = 3;
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 3)) & TIMER_MASK;
	}
	else if (qwDueTime < TimerIdType(1) << (TIMER_BITS_PER_WHEEL * (4 + 1)))
	{
		wheelIdx = 4;
		slotIdx = (qwExpires >> (TIMER_BITS_PER_WHEEL * 4)) & TIMER_MASK;
	}
	else if (qwDueTime < 0)
	{
		wheelIdx = 0;
		slotIdx = pTimerManager->qwCurrentTimeMS & TIMER_MASK;

#if defined( UNIQS_DEBUG_TIMER ) || defined (UNIQS_LOG_EVERYTHING)
		negative = true;
#endif
	}
	else
	{
		OnTimerError("AddTimer this should not happen");
		return;
	}

#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "AddTimer timerId:" << pTimer->qwTimerId << " source:" << pszAddTimerSource[source] << " fromWheelIdx:" << fromWheelIdx << " fromSlotIdx:" << fromSlotIdx <<
		" wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx << " negative:" << negative << " qwDueTime:" << qwDueTime <<
		" qwExpires:" << qwExpires << " qwCurrentTimeMS:" << pTimerManager->qwCurrentTimeMS;
#endif

#if defined (UNIQS_DEBUG_TIMER )
	printf("AddTimer source:%s fromWheelIdx:%llu fromSlotIdx:%llu wheelIdx:%llu slotIdx:%llu negative:%d qwDueTime:%llu qwExpires:%llu qwCurrentTimeMS:%llu\n",
		pszAddTimerSource[source], fromWheelIdx, fromSlotIdx, wheelIdx, slotIdx, int(negative), qwDueTime, qwExpires, pTimerManager->qwCurrentTimeMS);
#endif
	//printf("AddTimer qwExpires:%llu qwDueTime:%llu i:%llu pTimerManager->qwCurrentTimeMS:%llu\n", qwExpires, qwDueTime, i, pTimerManager->qwCurrentTimeMS);
	
	pTimer->wheelIdx = (int8_t)wheelIdx;
	pTimer->slotIdx = (uint8_t)slotIdx;
	pTimerManager->arrListTimer[wheelIdx][slotIdx].insert(pTimer);

	fucking[pTimer].push_back(std::make_tuple(EFucking_Add1, pTimer->qwTimerId, pTimerManager->qwCurrentTimeMS, __FILE__ + __LINE__ + std::to_string(wheelIdx) + std::to_string(slotIdx)));

}

// 循环该slot里的所有节点，重新AddTimer到管理器中
static void CascadeTimer(TimerManager* pTimerManager, TimerMsType wheelIdx, TimerMsType slotIdx)
{
#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "CascadeTimer begin wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif
#ifdef UNIQS_DEBUG_TIMER
	printf("CascadeTimer begin wheelIdx:%llu slotIdx:%llu \n", wheelIdx, slotIdx);
#endif

	auto& rSlot = pTimerManager->arrListTimer[wheelIdx][slotIdx];
	for (auto it = rSlot.begin();it != rSlot.end();)
	{
		TimerNode* pTimer = *it;
		if (pTimer->state & ETimerState_Running)
		{
#ifdef UNIQS_LOG_EVERYTHING
			LOG(INFO) << "CascadeTimer running AddTimer qwTimerId:" << pTimer->qwTimerId << " wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif
			extern TimerIdType timerIdRandStop;
			if (pTimer->qwTimerId < 0 || pTimer->qwTimerId >(timerIdRandStop + 100))
			{
				fuck(pTimer);
			}

			fucking[pTimer].push_back(std::make_tuple(EFucking_Cascading1, pTimer->qwTimerId, pTimerManager->qwCurrentTimeMS, __FILE__ + __LINE__));

			AddTimer(pTimerManager, pTimer, wheelIdx, slotIdx, EAddTimerSource_Cascade);
		}
		else
		{
#ifdef UNIQS_LOG_EVERYTHING
			LOG(INFO) << "CascadeTimer killed delete qwTimerId:" << pTimer->qwTimerId << " wheelIdx:" << wheelIdx << " slotIdx:" << slotIdx;
#endif

			fucking[pTimer].push_back(std::make_tuple(EFucking_Cascading2, pTimer->qwTimerId, pTimerManager->qwCurrentTimeMS, __FILE__ + __LINE__));

			fuck(pTimer);
			// timer还没被放到最终执行轮就已经被kill掉了，需要释放掉
			//pTimerManager->pTimers.erase(pTimer->qwTimerId);
			// 这里不需要从槽中删除，完了清槽
			OnTimerError("CascadeTimer this should not happen");
			//FreeObj(pTimer);
		}
		it = rSlot.erase(it);
	}
}

void TimerManager::Run()
{
	TimerMsType idxExecutingWheel = 0, idxNextWheel = 0, currTimeMS;

	currTimeMS = UTimerGetCurrentTimeMS();
	TimerIdType timerId = 0;
	while (currTimeMS >= qwCurrentTimeMS)
	{
		idxExecutingWheel = qwCurrentTimeMS & TIMER_MASK;

		idxNextWheel = idxExecutingWheel;
		int cascadeCount = 0;
		for (TimerMsType i = 0; i < 4 && idxNextWheel == 0; i++)
		{
			idxNextWheel = (qwCurrentTimeMS >> ((i + 1) * TIMER_BITS_PER_WHEEL)) & TIMER_MASK;
			CascadeTimer(this, i + 1, idxNextWheel);
			++cascadeCount;
		}

		auto& rSlot = arrListTimer[0][idxExecutingWheel];
		for (auto it = rSlot.begin();it != rSlot.end();)
		{
			TimerNode* pTimer = *it;

			timerId = pTimer->qwTimerId;
			if (pTimer->state & ETimerState_Running)
			{
#ifdef UNIQS_LOG_EVERYTHING
				LOG(INFO) << "pTimer->timerFn timerId:" << timerId << " idxExecutingWheel:" << idxExecutingWheel;
#endif
				pTimer->timerFn(pTimer->qwTimerId, pTimer->pParam);
#ifdef UNIQS_LOG_EVERYTHING
#endif
				if (pTimer->qwPeriod != 0)
				{
					pTimer->qwExpires = qwCurrentTimeMS + pTimer->qwPeriod;

					fucking[pTimer].push_back(std::make_tuple(EFucking_Run1, pTimer->qwTimerId, qwCurrentTimeMS, __FILE__ + __LINE__));

					// 不需要从槽中删除，完了会清槽
					AddTimer(this, pTimer, 0, idxExecutingWheel, EAddTimerSource_TimeoutReadd);
				}
				else
				{
#ifdef UNIQS_LOG_EVERYTHING
					LOG(INFO) << "TimerManager::Run running state. last trigger. kill delete timerId:" << timerId << " pTimer->qwPeriod:" <<
						pTimer->qwPeriod << " pTimer->qwExpires:" << pTimer->qwExpires << " currTimeMS:" << currTimeMS;
#endif

					fucking[pTimer].push_back(std::make_tuple(EFucking_Run2, pTimer->qwTimerId, qwCurrentTimeMS, __FILE__ + __LINE__));

					FreeObj(pTimer);
					// 不需要从槽中删除，完了会清槽
					pTimers.erase(timerId);
				}
			}
			else
			{
#ifdef UNIQS_LOG_EVERYTHING
				LOG(INFO) << "TimerManager::Run killed state. delete timerId:" << timerId << " pTimer->qwPeriod:" <<
					pTimer->qwPeriod << " pTimer->qwExpires:" << pTimer->qwExpires << " currTimeMS:" << currTimeMS;
#endif

				fucking[pTimer].push_back(std::make_tuple(EFucking_Run3, pTimer->qwTimerId, qwCurrentTimeMS, __FILE__ + __LINE__));

				FreeObj(pTimer);
			}
			it = rSlot.erase(it);
		}

		qwCurrentTimeMS++;
	}
}

TimerManager* CreateTimerManager(void)
{
	TimerManager* pTimerMgr = new TimerManager();
	if (pTimerMgr != NULL)
	{
		pTimerMgr->qwCurrentTimeMS = UTimerGetCurrentTimeMS();
		for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++)
		{
			for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++)
			{
			}
		}
	}
	return pTimerMgr;
}

void DestroyTimerManager(TimerManager* pTimerManager)
{
	if (NULL == pTimerManager)
		return;
	for (auto wheelIdx = 0; wheelIdx < TIMER_WHEEL_COUNT; wheelIdx++)
	{
		for (auto slotIdx = 0; slotIdx < TIMER_SLOT_COUNT_PER_WHEEL; slotIdx++)
		{
			for (auto it : pTimerManager->arrListTimer[wheelIdx][slotIdx])
			{
				FreeObj(it);
			}
		}
	}
	delete pTimerManager;
}

bool CreateTimer(TimerManager* pTimerManager, TimerIdType timerId, void(*timerFn)(TimerIdType, void*), void* pParam, TimerMsType qwDueTime, TimerMsType qwPeriod)
{
#ifdef UNIQS_LOG_EVERYTHING
	bool bParamOk = false;
	bool bExists = true;
#endif

	bool bOk = false;
	for (;;)
	{
		if (NULL == timerFn || NULL == pTimerManager)
			break;

#ifdef UNIQS_LOG_EVERYTHING
		LOG(INFO) << "CreateTimer timerId:" << timerId << " qwDueTime:" <<
			qwDueTime << " qwPeriod:" << qwPeriod << " pTimerManager->qwCurrentTimeMS:" << pTimerManager->qwCurrentTimeMS;
#endif
		// 两者都为0,无意义
		if (qwDueTime == 0 && qwPeriod == 0)
		{
			break;
		}

		// 如果创建重复性定时器，又设置触发时间为0,就直接把时间设置为重复时间
		if (qwDueTime == 0)
		{
			break;
		}

#ifdef UNIQS_LOG_EVERYTHING
		bParamOk = true;
#endif

		auto it = pTimerManager->pTimers.find(timerId);
		if (it != pTimerManager->pTimers.end())
		{
			break;
		}
#ifdef UNIQS_LOG_EVERYTHING
		bExists = false;
#endif

		TimerNode* pTimer = AllocObj();
		if (pTimer == nullptr)
		{
			OnTimerError("CreateTimer AllocObj failed.");
			break;
		}

		pTimer->qwPeriod = qwPeriod;
		pTimer->timerFn = timerFn;
		pTimer->pParam = pParam;
		pTimer->qwTimerId = timerId;

		pTimer->state = ETimerState_Running;

		fucking[pTimer].push_back(std::make_tuple(EFucking_Crateing, timerId, pTimerManager->qwCurrentTimeMS, __FILE__ + __LINE__ + std::to_string(timerId)));

		pTimer->qwExpires = pTimerManager->qwCurrentTimeMS + qwDueTime;
		AddTimer(pTimerManager, pTimer, 0, 0, EAddTimerSource_Create_NewAlloc);
		pTimerManager->pTimers[timerId] = pTimer;

		bOk = true;
		break;
	}

#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "CreateTimer timerId:" << timerId << " bOk:" << bOk << " bParamOk:" << bParamOk << " exists:" << bExists;
#endif

	return bOk;
}

bool KillTimer(TimerManager* pTimerManager, TimerIdType timerId)
{
#ifdef UNIQS_LOG_EVERYTHING
	bool bExists = false;
	bool bInExecutingWheel = true;
#endif

	bool bOk = false;
	uint8_t wheelIdx = 0;
	uint8_t slotIdx = 0;
	for (;;)
	{
		if (pTimerManager == nullptr)
		{
			break;
		}
		auto it = pTimerManager->pTimers.find(timerId);
		if (it == pTimerManager->pTimers.end())
		{
			break;
		}

#ifdef UNIQS_LOG_EVERYTHING
		bExists = true;
#endif

		TimerNode* pTimer = it->second;
		if (!(pTimer->state & ETimerState_Running))
		{
			break;
		}

		// 优化，如果不在第一个轮上，可以立即释放掉
		if (pTimer->wheelIdx > 0)
		{
#ifdef UNIQS_LOG_EVERYTHING
			bInExecutingWheel = false;
#endif
			wheelIdx = pTimer->wheelIdx;
			slotIdx = pTimer->slotIdx;
			pTimerManager->pTimers.erase(it);
			pTimerManager->arrListTimer[wheelIdx][slotIdx].erase(pTimer);
			FreeObj(pTimer);

			fucking[pTimer].push_back(std::make_tuple(EFucking_Killing1, timerId, pTimerManager->qwCurrentTimeMS, __FILE__ + __LINE__));
		}
		else
		{
			pTimer->state = ETimerState_Killed;
			pTimerManager->pTimers.erase(it);

			fucking[pTimer].push_back(std::make_tuple(EFucking_Killing2, timerId, pTimerManager->qwCurrentTimeMS, __FILE__ + __LINE__));
		}

		bOk = true;
		break;
	}

#ifdef UNIQS_LOG_EVERYTHING
	LOG(INFO) << "KillTimer timerId:" << timerId << " bOk:" << bOk << " exists:" << bExists << " bInExecutingWheel:" << bInExecutingWheel
		<< " wheelIdx:"<< wheelIdx << " slotIdx:" << slotIdx;
#endif

	return bOk;
}
