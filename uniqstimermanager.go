package uniqstimer

type UTimerWheelLevel uint8

const (
	UTimerWheelLevelUS UTimerWheelLevel = iota
	UTimerWheelLevelMS
	UTimerWheelLevelS
	UTimerWheelLevelH
	UTimerWheelLevelDay
	UTimerWheelLevelMax
)

const SlotsCountUS = 1000
const SlotsCountMS = 1000
const SlotsCountS = 3600
const SlotsCountH = 240
const SlotsCountDay = 1000

type UTimerManager struct {
	slotsUS  [SlotsCountUS]_UTimerSlot
	slotsMS  [SlotsCountMS]_UTimerSlot
	slotsS   [SlotsCountS]_UTimerSlot
	slotsH   [SlotsCountH]_UTimerSlot
	slotsDay [SlotsCountDay]_UTimerSlot
	idxUS    int64
	idxMS    int64
	idxS     int64
	idxH     int64
	idxDay   int64
}

func (mgr *UTimerManager) Update(deltaUS int64) {
	passUS := deltaUS % SlotsCountUS
	deltaUS /= SlotsCountUS
	passMS := deltaUS % SlotsCountMS
	deltaUS /= SlotsCountMS
	passS := deltaUS % SlotsCountS
	deltaUS /= SlotsCountS
	passH := deltaUS % SlotsCountH
	deltaUS /= SlotsCountH
	passDay := deltaUS % SlotsCountDay
}

func NewUTimerManager() *UTimerManager {
	ret := &UTimerManager{}
	return ret
}
