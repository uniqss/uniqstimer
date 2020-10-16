#include "timer_mem.h"

TimerNode* AllocObj()
{
	auto ret = new TimerNode();
	return ret;
}

void FreeObj(TimerNode* pTimer)
{
	delete pTimer;
}
