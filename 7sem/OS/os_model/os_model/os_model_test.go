package os_model

import (
	"testing"

	"os_model/process"
)

func TestCPUStateFollowsScheduler(t *testing.T) {
	m := New()

	if m.CPU.State != CPUStateWait {
		t.Fatalf("initial CPU state = %q, want %q", m.CPU.State, CPUStateWait)
	}

	// Пустая очередь готовности — ЦПр ожидает.
	m.UpdateCPUState()
	if m.CPU.State != CPUStateWait {
		t.Fatalf("idle CPU state = %q, want %q", m.CPU.State, CPUStateWait)
	}

	// Есть активный процесс — ЦПр работает.
	m.Scheduler.PlaceTask(process.PSW{ID: 1, Size: 100, State: process.StateActive, Prior: 1})
	m.Scheduler.ActiveIndex = 0
	m.UpdateCPUState()
	if m.CPU.State != CPUStateWork {
		t.Fatalf("busy CPU state = %q, want %q", m.CPU.State, CPUStateWork)
	}
}
