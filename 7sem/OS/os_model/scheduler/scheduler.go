// Package scheduler реализует планировщик (диспетчер задач):
// таблицу процессов, дисциплину диспетчеризации и примитивы смены контекста.
package scheduler

import (
	"fmt"
	"strings"

	"os_model/process"
)

// ----------------------------------------------------------------------------------
// Настройки планировщика
const (
	// DefaultTimeQuantum — базовый размер кванта времени (в тактах).
	DefaultTimeQuantum = 10
	// PriorityStepDivisor — делитель шага динамического приоритета:
	// шаг = max(1, Size / PriorityStepDivisor). Чем больше задание,
	// тем быстрее растёт его приоритет.
	PriorityStepDivisor = 50
	// DefaultSlots — число слотов в таблице процессов.
	DefaultSlots = 16
)

// ----------------------------------------------------------------------------------
// Scheduler — планировщик (диспетчер задач).
type Scheduler struct {
	Table       []process.PSW // таблица процессов (слов состояний)
	ActiveIndex int           // номер активного процесса, -1 если ЦПр свободен

	TimeQuantum int // базовый размер кванта времени
	QuantumLeft int // остаток кванта времени текущего процесса

	Formula string // строковое представление формулы выбора процесса
}

// ----------------------------------------------------------------------------------
// New создаёт планировщик с заданным числом слотов и размером кванта.
func New(slots, quantum int) *Scheduler {
	if slots <= 0 {
		slots = DefaultSlots
	}

	if quantum <= 0 {
		quantum = DefaultTimeQuantum
	}

	s := &Scheduler{
		Table:       make([]process.PSW, slots),
		ActiveIndex: -1,
		TimeQuantum: quantum,
		Formula:     "Планировщик не запускался",
	}

	for i := range s.Table {
		s.Table[i] = process.PSW{State: process.StateAbsent}
	}

	return s
}

// ----------------------------------------------------------------------------------
// Reset очищает таблицу процессов и сбрасывает состояние планировщика.
func (s *Scheduler) Reset() {
	for i := range s.Table {
		s.Table[i] = process.PSW{State: process.StateAbsent}
	}
	s.ActiveIndex = -1
	s.QuantumLeft = 0
	s.Formula = "Ожидание готовых процессов"
}

// ----------------------------------------------------------------------------------
// UsedMemory возвращает суммарный размер присутствующих процессов.
func (s *Scheduler) UsedMemory() int {
	used := 0
	for _, p := range s.Table {
		if p.Present() {
			used += p.Size
		}
	}
	return used
}

// ----------------------------------------------------------------------------------
// FindFreeSlot возвращает индекс свободного слота или -1, если мест нет.
func (s *Scheduler) FindFreeSlot() int {
	for i := range s.Table {
		if s.Table[i].State == process.StateAbsent {
			return i
		}
	}
	return -1
}

// ----------------------------------------------------------------------------------
// PlaceTask размещает задание в первом свободном слоте.
func (s *Scheduler) PlaceTask(p process.PSW) bool {
	slot := s.FindFreeSlot()
	if slot < 0 {
		return false
	}

	s.Table[slot] = p

	return true
}

// ----------------------------------------------------------------------------------
// Примитивы смены контекста

// SaveProcessState сохраняет значение счетчика команд ЦПр в PSW процесса,
// который покидает процессор.
func (s *Scheduler) SaveProcessState(index, pc int) {
	if index < 0 || index >= len(s.Table) {
		return
	}
	s.Table[index].PC = pc
}

// ----------------------------------------------------------------------------------
// RestoreProcessState возвращает значение PC из PSW выбранного процесса,
// которое затем загружается в аппаратный счетчик команд ЦПр.
func (s *Scheduler) RestoreProcessState(index int) int {
	if index < 0 || index >= len(s.Table) {
		return 0
	}
	return s.Table[index].PC
}

// ----------------------------------------------------------------------------------
// Ядро планировщика (11 вариант)
// «Относительные и динамические приоритеты, приоритет для больших заданий растёт»

// GetNextProcessForCPU выбирает следующий процесс из списка готовности.
//
// Относительные приоритеты: работающий процесс не прерывается — выбор выполняется
// только при вызове планировщика (конец кванта или блокировка).
//
// Динамические приоритеты: перед выбором приоритет всех готовых процессов
// увеличивается. Шаг увеличения пропорционален размеру задания, поэтому у больших
// заданий приоритет растёт быстрее.
func (s *Scheduler) GetNextProcessForCPU() int {
	bestIndex := -1
	bestPrior := -1

	var steps []string

	for i := range s.Table {
		if s.Table[i].State != process.StateReady {
			continue
		}

		// Шаг пропорционален размеру задания (не меньше единицы).
		step := s.Table[i].Size / PriorityStepDivisor
		if step < 1 {
			step = 1
		}

		s.Table[i].Prior += step
		steps = append(steps, fmt.Sprintf("P%d+%d", i, step))

		if s.Table[i].Prior > bestPrior {
			bestPrior = s.Table[i].Prior
			bestIndex = i
		}
	}

	if bestIndex == -1 {
		s.Formula = "Нет готовых процессов — ЦПр простаивает (Ожидание)"
		return -1
	}

	s.Formula = fmt.Sprintf(
		"P[i] += max(1, Size[i]/%d); выбран слот %d (ID %d), P=%d | %s",
		PriorityStepDivisor, bestIndex, s.Table[bestIndex].ID, bestPrior,
		strings.Join(steps, " "),
	)
	return bestIndex
}

// ----------------------------------------------------------------------------------
// SelectNextProcess выбирает новый активный процесс и восстанавливает его
// контекст. Возвращает актуальное значение аппаратного счетчика команд: либо
// восстановленное из PSW выбранного процесса, либо переданное (если очередь пуста).
func (s *Scheduler) SelectNextProcess(pc int) int {
	next := s.GetNextProcessForCPU()

	if next < 0 {
		s.ActiveIndex = -1
		s.QuantumLeft = 0
		return pc
	}

	s.Table[next].State = process.StateActive
	s.ActiveIndex = next

	// RestoreProcessState уже возвращает PC выбранного процесса — отдельное
	// присваивание ему обратно не требуется.
	pc = s.RestoreProcessState(next)

	s.QuantumLeft = s.TimeQuantum
	if s.QuantumLeft < 1 {
		s.QuantumLeft = 1
	}
	return pc
}

// ----------------------------------------------------------------------------------
