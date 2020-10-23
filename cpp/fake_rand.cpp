#include "fake_rand.h"
#include <stdlib.h>

#include "mersenne_rand.hpp"

#include "main.h"


const int FakeRandCount = 20000;
int FakeRandArr[FakeRandCount];
int FakeRandCurr = 0;

void FakeRandInit()
{
	mtsrand(UTimerGetCurrentTimeUS());

	for (int i = 0; i < FakeRandCount; i++)
	{
		FakeRandArr[i] = rand();
	}
}

size_t FakeRand()
{
	return mtirand();
	++FakeRandCurr;
	return FakeRandCurr;
	if (FakeRandCurr >= FakeRandCount)
	{
		FakeRandCurr = 0;
	}
	return FakeRandArr[FakeRandCurr];
}
