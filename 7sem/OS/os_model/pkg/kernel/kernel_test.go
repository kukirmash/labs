package kernel

import (
	"testing"
	"time"

	"os_model/pkg/process"
	"os_model/pkg/scheduler"
)

func TestTerminateProcessFreesResources(t *testing.T) {
	m := New()
	m.InitModel()

	task := process.PSW{ID: 7, Size: 50, State: process.StateReady, Prior: 1, TotalCommands: 5}
	if !m.Memory.Allocate(task.Size, task.ID) {
		t.Fatal("failed to allocate memory for the task")
	}
	if !m.Scheduler.PlaceTask(task) {
		t.Fatal("failed to place the task")
	}

	m.TerminateProcess(task.ID)

	if idx := m.Scheduler.FindProcess(task.ID); idx != -1 {
		t.Fatalf("process still present in slot %d", idx)
	}
	if got := m.Memory.GetUsed(); got != 0 {
		t.Fatalf("used memory = %d, want 0", got)
	}
	if m.CompletedTasks != 1 {
		t.Fatalf("completed tasks = %d, want 1", m.CompletedTasks)
	}
}

func TestRequestIOAndComplete(t *testing.T) {
	m := New()
	m.InitModel()

	task := process.PSW{
		ID:             3,
		Size:           10,
		State:          process.StateActive,
		Prior:          1,
		TotalCommands:  5,
		IORequestTicks: 2,
	}
	if !m.Scheduler.PlaceTask(task) {
		t.Fatal("failed to place the task")
	}
	m.Scheduler.ActiveIndex = 0

	psw := &m.Scheduler.Table[0]
	m.RequestIO(psw)

	if psw.State != process.StateBlockIO {
		t.Fatalf("state = %q, want %q", psw.State, process.StateBlockIO)
	}
	if !m.IO.Busy() || m.IO.TicksLeft != 2 {
		t.Fatalf("io busy=%v left=%d, want busy with 2 ticks", m.IO.Busy(), m.IO.TicksLeft)
	}

	m.IO.Tick(m)
	if !m.IO.Busy() {
		t.Fatal("IO must still be in progress after one tick")
	}

	m.IO.Tick(m) // здесь срабатывает системный вызов IOComplete
	if m.IO.Busy() {
		t.Fatal("IO device must be free after completion")
	}
	if psw.State != process.StateReady {
		t.Fatalf("state = %q, want %q", psw.State, process.StateReady)
	}
	if psw.IORequestTicks != 0 {
		t.Fatalf("io request ticks = %d, want 0", psw.IORequestTicks)
	}
}

func TestBuildSnapshotContainsSubsystems(t *testing.T) {
	m := New()
	m.InitModel()
	m.LoadTasks()

	snapshot := m.BuildSnapshot()

	if len(snapshot.Processes) != scheduler.DefaultSlots {
		t.Fatalf("processes = %d, want %d slots", len(snapshot.Processes), scheduler.DefaultSlots)
	}
	if snapshot.Memory.Total != MemorySize {
		t.Fatalf("memory total = %d, want %d", snapshot.Memory.Total, MemorySize)
	}
	if snapshot.Memory.Used == 0 {
		t.Fatal("expected resident tasks to occupy memory")
	}
	if snapshot.CommandText == "" {
		t.Fatal("command text must not be empty")
	}
	if snapshot.ResidentTasks == 0 {
		t.Fatal("expected at least one resident task")
	}
}

func TestStartStopsAndEmitsSnapshots(t *testing.T) {
	m := New()
	m.CPU.Speed = 1000 // 1 мс на такт, чтобы тест не ждал

	snapshots := make(chan Snapshot, 1)
	m.EmitUpdate = func(s Snapshot) {
		select {
		case snapshots <- s:
		default:
		}
	}

	go m.Start()

	select {
	case s := <-snapshots:
		if s.Speed != 1000 {
			t.Fatalf("speed = %v, want 1000", s.Speed)
		}
	case <-time.After(3 * time.Second):
		t.Fatal("no snapshots received from the simulation loop")
	}

	m.Stop()
}
