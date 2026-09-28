package scheduler

import (
	"testing"

	"os_model/pkg/process"
)

func TestGetNextProcessForCPU(t *testing.T) {
	s := New(16, DefaultTimeQuantum)
	s.PlaceTask(process.PSW{ID: 1, Size: 60, State: process.StateReady, Prior: 2})
	s.PlaceTask(process.PSW{ID: 2, Size: 250, State: process.StateReady, Prior: 3})
	s.PlaceTask(process.PSW{ID: 3, Size: 40, State: process.StateReady, Prior: 1})

	before := make([]process.PSW, len(s.Table))
	copy(before, s.Table)

	next := s.GetNextProcess()
	if next < 0 {
		t.Fatal("scheduler returned no process")
	}

	bestPrior := -1
	bestIndex := -1
	for i := range s.Table {
		if before[i].State != process.StateReady {
			continue
		}
		wantStep := before[i].Size / PriorityStepDivisor
		if wantStep < 1 {
			wantStep = 1
		}
		if got := s.Table[i].Prior - before[i].Prior; got != wantStep {
			t.Fatalf("slot %d: priority step = %d, want %d", i, got, wantStep)
		}
		if s.Table[i].Prior > bestPrior {
			bestPrior = s.Table[i].Prior
			bestIndex = i
		}
	}
	if next != bestIndex {
		t.Fatalf("chosen slot %d, want %d (max priority)", next, bestIndex)
	}
	if s.Formula == "" {
		t.Fatal("formula is empty")
	}
}

func TestSelectNextProcessAndPrimitives(t *testing.T) {
	s := New(16, 5)
	s.PlaceTask(process.PSW{ID: 1, Size: 100, PC: 7, State: process.StateReady, Prior: 1})

	idx := s.SelectNextProcess()
	if idx != 0 || s.ActiveIndex != 0 {
		t.Fatalf("selected index = %d, active = %d, want 0", idx, s.ActiveIndex)
	}
	if pc := s.RestoreProgramCounter(0); pc != 7 {
		t.Fatalf("restored PC = %d, want 7", pc)
	}
	if s.Table[0].State != process.StateActive {
		t.Fatalf("state = %q, want %q", s.Table[0].State, process.StateActive)
	}
	if s.Table[0].Prior != 0 {
		t.Fatalf("prior = %d, want 0 (reset on dispatch)", s.Table[0].Prior)
	}
	if s.QuantumLeft != 5 {
		t.Fatalf("quantum = %d, want 5", s.QuantumLeft)
	}

	s.SaveProgramCounter(0, 99)
	if s.Table[0].PC != 99 {
		t.Fatalf("saved PC = %d, want 99", s.Table[0].PC)
	}
}

func TestSelectNextProcessIdle(t *testing.T) {
	s := New(2, 5)

	idx := s.SelectNextProcess()
	if idx != -1 {
		t.Fatalf("selected index = %d, want -1", idx)
	}
	if s.ActiveIndex != -1 {
		t.Fatalf("active index = %d, want -1", s.ActiveIndex)
	}
	if s.QuantumLeft != 0 {
		t.Fatalf("quantum = %d, want 0", s.QuantumLeft)
	}
}

func TestFindProcessAndClearSlot(t *testing.T) {
	s := New(4, 5)
	s.PlaceTask(process.PSW{ID: 10, Size: 50, State: process.StateReady})
	s.PlaceTask(process.PSW{ID: 20, Size: 50, State: process.StateReady})

	if got := s.FindProcess(20); got != 1 {
		t.Fatalf("find(20) = %d, want 1", got)
	}
	if got := s.FindProcess(99); got != -1 {
		t.Fatalf("find(99) = %d, want -1", got)
	}
	if got := s.GetResidentCount(); got != 2 {
		t.Fatalf("resident = %d, want 2", got)
	}

	s.ClearSlot(1)
	if got := s.FindProcess(20); got != -1 {
		t.Fatalf("find(20) after clear = %d, want -1", got)
	}
	if got := s.GetResidentCount(); got != 1 {
		t.Fatalf("resident after clear = %d, want 1", got)
	}
}
