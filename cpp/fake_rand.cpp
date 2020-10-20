#include "fake_rand.h"
#include <stdlib.h>

const int FakeRandCount = 20000;
int FakeRandArr[FakeRandCount];
int FakeRandCurr = 0;

void FakeRandInit()
{
	for (int i = 0; i < FakeRandCount; i++)
	{
		FakeRandArr[i] = rand();
	}
}

int FakeRand()
{
	++FakeRandCurr;
	if (FakeRandCurr >= FakeRandCount)
	{
		FakeRandCurr = 0;
	}
	return FakeRandArr[FakeRandCurr];
}
