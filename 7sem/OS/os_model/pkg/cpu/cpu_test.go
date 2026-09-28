package cpu

import (
	"testing"

	"os_model/pkg/process"
)

// fakeSys — тестовый обработчик системных вызовов, фиксирующий обращения АЛУ.
type fakeSys struct {
	requested    *process.PSW
	terminatedID int
}

func (f *fakeSys) RequestIO(psw *process.PSW) {
	f.requested = psw
}

func (f *fakeSys) TerminateProcess(pswID int) {
	f.terminatedID = pswID
}

func TestTickCompute(t *testing.T) {
	c := New()
	p := &process.PSW{ID: 1, State: process.StateActive, TotalCommands: 3, IOProbability: 0}

	c.Tick(p, nil)

	if c.LastCommand.Code != OpCompute {
		t.Fatalf("command = %v, want %v", c.LastCommand.Code, OpCompute)
	}
	if p.PC != 1 {
		t.Fatalf("pc = %d, want 1", p.PC)
	}
	if !c.HasCommand {
		t.Fatal("HasCommand = false, want true")
	}
	if want := c.LastCommand.Arg1 + c.LastCommand.Arg2; p.LastResult != want {
		t.Fatalf("result = %d, want %d", p.LastResult, want)
	}
	if c.State != CPUStateWork {
		t.Fatalf("cpu state = %q, want %q", c.State, CPUStateWork)
	}
}

func TestTickRequestsIO(t *testing.T) {
	c := New()
	sys := &fakeSys{}
	p := &process.PSW{ID: 2, State: process.StateActive, TotalCommands: 5, IOProbability: 1.0}

	c.Tick(p, sys)

	if sys.requested != p {
		t.Fatal("ALU did not pass the process to the system call handler")
	}
	if p.State != process.StateInitIO {
		t.Fatalf("state = %q, want %q", p.State, process.StateInitIO)
	}
	if p.IORequestTicks < 1 || p.IORequestTicks > process.MaxIOBurst {
		t.Fatalf("io ticks = %d, want 1..%d", p.IORequestTicks, process.MaxIOBurst)
	}
}

func TestTickTerminatesProcess(t *testing.T) {
	c := New()
	sys := &fakeSys{}
	p := &process.PSW{ID: 3, State: process.StateActive, TotalCommands: 0}

	c.Tick(p, sys)

	if sys.terminatedID != 3 {
		t.Fatalf("terminated id = %d, want 3", sys.terminatedID)
	}
	if p.State != process.StateEndIO {
		t.Fatalf("state = %q, want %q", p.State, process.StateEndIO)
	}
}

func TestTickIdleWithoutActiveProcess(t *testing.T) {
	c := New()
	p := &process.PSW{ID: 4, State: process.StateReady}

	c.Tick(p, nil)

	if c.State != CPUStateWait {
		t.Fatalf("cpu state = %q, want %q", c.State, CPUStateWait)
	}
	if p.PC != 0 {
		t.Fatalf("ready process pc changed to %d", p.PC)
	}
}
