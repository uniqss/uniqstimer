#include <cstddef>
#include "fake_rand.h"
#include <stdlib.h>

#include "mersenne_rand.h"

#include "main.h"

void FakeRandInit() {
    mtsrand(UTimerGetCurrentTimeUS());
}

size_t FakeRand() {
    return mtrandi();
}
