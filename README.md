# uniqstimer C++/go/js

## uniqstimer:
if needed, change these constants to set the timer wheel and slots:
* TIMER_BITS_PER_WHEEL: every wheel's bits. the wheel's size is 2^TIMER_BITS_PER_WHEEL
* TIMER_WHEEL_COUNT: wheel count.

by default, TIMER_BITS_PER_WHEEL is set to 10 and TIMER_WHEEL_COUNT is set to 4.

by default, 2^10^4MS = 12,725.8 days = 34.8 years  supported.

## usage:
index | language | usage
------ | ------ | ------
I. | cpp | just copy all files in directory cpp into your project
II. | go | import "github.com/uniqss/uniqstimer"
III. | js | just copy all files in directory js into your project

## cpp test:
```
cd cpp_test
sh b.sh
./a.out
```
if you don't have gperftools, remove [-lprofiler -ltcmalloc] in b.sh

test result:

timerTickMs | %CPU | average(microseconds) | %cost cpu percent
------ | ------ | ------ | ------
1ms | 9 | 90 | 90/1000
10ms | 5 | 450 | 450/10000
20ms | 4 | 750 | 750/20000
50ms | 3 | 1630 | 1630/50000
