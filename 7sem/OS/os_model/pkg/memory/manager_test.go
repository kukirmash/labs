package memory

import (
	"reflect"
	"testing"
)

func TestAllocateFirstFitAndSplit(t *testing.T) {
	m := New(100)

	if !m.Allocate(30, 1) {
		t.Fatal("allocate 30 for P1 failed")
	}
	if !m.Allocate(20, 2) {
		t.Fatal("allocate 20 for P2 failed")
	}

	want := []Allocation{
		{ProcessID: 1, Start: 0, Size: 30},
		{ProcessID: 2, Start: 30, Size: 20},
	}
	if got := m.GetAllocations(); !reflect.DeepEqual(got, want) {
		t.Fatalf("allocations = %+v, want %+v", got, want)
	}

	if got := m.GetUsed(); got != 50 {
		t.Fatalf("used = %d, want 50", got)
	}
	if got := m.FreeMemory(); got != 50 {
		t.Fatalf("free = %d, want 50", got)
	}

	free := m.GetFreeBlocks()
	if len(free) != 1 || free[0] != (Block{Start: 50, Size: 50}) {
		t.Fatalf("free blocks = %+v, want [{50 50}]", free)
	}
}

func TestFreeCoalescesNeighbours(t *testing.T) {
	m := New(100)
	m.Allocate(10, 1)
	m.Allocate(20, 2)
	m.Allocate(30, 3)

	// Освобождение среднего блока не склеивает соседей.
	m.Free(2)
	if free := m.GetFreeBlocks(); len(free) != 2 {
		t.Fatalf("free blocks after middle free = %+v, want 2 blocks", free)
	}

	// Освобождение первого блока склеивает его со средним.
	m.Free(1)
	free := m.GetFreeBlocks()
	if len(free) != 2 || free[0] != (Block{Start: 0, Size: 30}) {
		t.Fatalf("free blocks after left free = %+v, want [{0 30} {60 40}]", free)
	}

	// Освобождение последнего блока склеивает всю память в один блок.
	m.Free(3)
	free = m.GetFreeBlocks()
	if len(free) != 1 || free[0] != (Block{Start: 0, Size: 100}) {
		t.Fatalf("free blocks after all free = %+v, want [{0 100}]", free)
	}
	if got := m.GetUsed(); got != 0 {
		t.Fatalf("used = %d, want 0", got)
	}
}

func TestAllocateFails(t *testing.T) {
	m := New(100)

	if !m.Allocate(60, 1) {
		t.Fatal("first allocation failed")
	}
	if m.Allocate(50, 2) {
		t.Fatal("allocation of 50 with only 40 free must fail")
	}
	if m.Allocate(10, 1) {
		t.Fatal("duplicate allocation for the same process must fail")
	}
	if m.Allocate(0, 3) || m.Allocate(-5, 4) {
		t.Fatal("non-positive allocation must fail")
	}
}

func TestStatsAndReset(t *testing.T) {
	m := New(100)
	m.Allocate(25, 7)

	stats := m.GetStats()
	if stats.Total != 100 || stats.Used != 25 || stats.Free != 75 {
		t.Fatalf("stats = %+v, want total 100, used 25, free 75", stats)
	}
	if stats.Fragments != 1 {
		t.Fatalf("fragments = %d, want 1", stats.Fragments)
	}
	if len(stats.UsedBlocks) != 1 || stats.UsedBlocks[0].ProcessID != 7 {
		t.Fatalf("used blocks = %+v, want one block of P7", stats.UsedBlocks)
	}

	m.Reset()
	if got := m.GetUsed(); got != 0 {
		t.Fatalf("used after reset = %d, want 0", got)
	}
	if free := m.GetFreeBlocks(); len(free) != 1 || free[0].Size != 100 {
		t.Fatalf("free blocks after reset = %+v, want [{0 100}]", free)
	}
}
