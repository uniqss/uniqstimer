package uniqstimer

type _UTimerWheel struct {
	Slots []*_UTimerSlot
}

func NewTimerWheel(count int) *_UTimerWheel {
	ret := &_UTimerWheel{}
	for i:=0;i < count;i++ {
		ret.Slots = append(ret.Slots, NewTimerSlot())
	}

	return ret
}
