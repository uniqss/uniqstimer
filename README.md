# uniqstimer C++/go/js

五个轮
每个轮里面有256个槽
精度1MS

总共256*5=1280个槽
总共是2^40=1,099,511,627,776MS
=12,725.829天
=34.865年

34年，应该一般的场景够用了。适用于时间精度要求非常高的场景(精度毫秒)，比如帧同步里的BUFF。

uniqstimerIII:
五个轮
每个轮里面有64个槽
精度100MS

总共64*5=320个槽
总共是2^30= ‭1,073,741,824‬ 单位：百毫秒
=‭107,374,182.4‬秒
=‭1,242.757天
=3.4年
3年多，一般的场景够用。适用于时间要求精度不是非常高(100MS以内)的场景。比如各种排行榜结算、比如玩家隔天凌晨4点刷新、比如月卡、比如心跳、比如登录空SESSION超时等等。

TIMER有一个问题：如果频繁删除时间非常非常长的定时器，会导致进程的内存会越来越多
有很多定时器被标记为Killed状态，躺在高级的时间轮里面，要轮到的时候才会清。

如果是在MMO里用，添加TIMER的时候尽量避开非常频繁地删除时间特别长的定时器(持续特别长但中间会触发的不算在此列，这里是指首次触发就特别长/或者触发后再度触发的时间特别长且被触发过)

相同的TimerId不能重复
可以在_helper里面自己定义用池或者是定义分配和释放策略

测试先开一个根节点timer，然后在这个根节点的OnTimer里面创建母Timer，
	再在母timer里面随机删除和创建定时器。
测试有一定的时间误差，时间精度为100MS
