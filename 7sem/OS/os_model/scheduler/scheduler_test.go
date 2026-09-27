package scheduler

import (
	"testing"

	"os_model/process"
)

func TestGetNextProcessForCPU(t *testing.T) {
	s := New(16, DefaultTimeQuantum)
	s.PlaceTask(process.PSW{ID: 1, Size: 60, State: process.StateReady, Prior: 2})
	s.PlaceTask(process.PSW{ID: 2, Size: 250, State: process.StateReady, Prior: 3})
	s.PlaceTask(process.PSW{ID: 3, Size: 40, State: process.StateReady, Prior: 1})

	before := make([]process.PSW, len(s.Table))
	copy(before, s.Table)

	next := s.GetNextProcessForCPU()
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

	pc := s.SelectNextProcess(0)
	if s.ActiveIndex != 0 {
		t.Fatalf("active index = %d, want 0", s.ActiveIndex)
	}
	if pc != 7 {
		t.Fatalf("restored PC = %d, want 7", pc)
	}
	if s.Table[0].State != process.StateActive {
		t.Fatalf("state = %q, want %q", s.Table[0].State, process.StateActive)
	}
	if s.QuantumLeft != 5 {
		t.Fatalf("quantum = %d, want 5", s.QuantumLeft)
	}

	s.SaveProcessState(0, 99)
	if s.Table[0].PC != 99 {
		t.Fatalf("saved PC = %d, want 99", s.Table[0].PC)
	}
}

func TestSelectNextProcessIdle(t *testing.T) {
	s := New(2, 5)
	pc := s.SelectNextProcess(42)
	if s.ActiveIndex != -1 {
		t.Fatalf("active index = %d, want -1", s.ActiveIndex)
	}
	if pc != 42 {
		t.Fatalf("pc = %d, want 42", pc)
	}
}
