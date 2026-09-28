package io

import (
	"testing"

	"os_model/pkg/process"
)

// fakeSys — тестовый обработчик прерывания завершения ввода-вывода.
type fakeSys struct {
	done *process.PSW
}

func (f *fakeSys) IOComplete(psw *process.PSW) {
	f.done = psw
}

func TestTickCountsDownAndCompletes(t *testing.T) {
	dev := New()
	sys := &fakeSys{}
	psw := &process.PSW{ID: 5, IORequestTicks: 3}

	dev.Start(psw)
	if !dev.Busy() || dev.TicksLeft != 3 || dev.TotalTicks != 3 {
		t.Fatalf("after start: busy=%v left=%d total=%d", dev.Busy(), dev.TicksLeft, dev.TotalTicks)
	}

	dev.Tick(sys)
	dev.Tick(sys)
	if sys.done != nil {
		t.Fatal("IO completed too early")
	}
	if dev.TicksLeft != 1 {
		t.Fatalf("ticks left = %d, want 1", dev.TicksLeft)
	}

	dev.Tick(sys)
	if sys.done != psw {
		t.Fatal("IO completion handler was not called with the active process")
	}
	if dev.Busy() {
		t.Fatal("device is still busy after completion")
	}
}

func TestStartUsesDefaultDuration(t *testing.T) {
	dev := New()
	dev.Start(&process.PSW{ID: 6})

	if dev.TotalTicks != DefaultDuration {
		t.Fatalf("total ticks = %d, want %d", dev.TotalTicks, DefaultDuration)
	}
}

func TestTickIdleIsNoop(t *testing.T) {
	dev := New()
	dev.Tick(nil) // не должно паниковать

	if dev.Busy() || dev.TicksLeft != 0 {
		t.Fatalf("idle device changed state: busy=%v left=%d", dev.Busy(), dev.TicksLeft)
	}
}

func TestStateSnapshot(t *testing.T) {
	dev := New()
	dev.Start(&process.PSW{ID: 9, IORequestTicks: 2})

	state := dev.State()
	if !state.Busy || state.ProcessID != 9 || state.TicksLeft != 2 || state.TotalTicks != 2 {
		t.Fatalf("state = %+v", state)
	}
}
