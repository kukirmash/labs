// Package scheduler реализует планировщик (диспетчер задач):
// таблицу процессов, дисциплину диспетчеризации и примитивы смены контекста.
package scheduler

import (
	"fmt"
	"strings"

	"os_model/pkg/process"
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
func (sch *Scheduler) Reset() {
	for i := range sch.Table {
		sch.Table[i] = process.PSW{State: process.StateAbsent}
	}
	sch.ActiveIndex = -1
	sch.QuantumLeft = 0
	sch.Formula = "Ожидание готовых процессов"
}

// ----------------------------------------------------------------------------------
// FindFreeSlot возвращает индекс свободного слота или -1, если мест нет.
func (sch *Scheduler) FindFreeSlot() int {
	for i := range sch.Table {
		if sch.Table[i].State == process.StateAbsent {
			return i
		}
	}
	return -1
}

// ----------------------------------------------------------------------------------
// PlaceTask размещает задание в первом свободном слоте.
func (sch *Scheduler) PlaceTask(psw process.PSW) bool {
	slot := sch.FindFreeSlot()
	if slot < 0 {
		return false
	}

	sch.Table[slot] = psw

	return true
}

// ----------------------------------------------------------------------------------
// FindProcess возвращает индекс слота процесса с заданным ID или -1.
func (sch *Scheduler) FindProcess(id int) int {
	for i := range sch.Table {
		if sch.Table[i].IsPresent() && sch.Table[i].ID == id {
			return i
		}
	}
	return -1
}

// ----------------------------------------------------------------------------------
// ClearSlot освобождает слот таблицы процессов (процесс завершён).
func (sch *Scheduler) ClearSlot(index int) {
	if index < 0 || index >= len(sch.Table) {
		return
	}

	sch.Table[index] = process.PSW{State: process.StateAbsent}

	if sch.ActiveIndex == index {
		sch.ActiveIndex = -1
	}
}

// ----------------------------------------------------------------------------------
// GetResidentCount возвращает число процессов, находящихся в памяти.
func (sch *Scheduler) GetResidentCount() int {
	count := 0
	for i := range sch.Table {
		if sch.Table[i].IsPresent() {
			count++
		}
	}
	return count
}

// ----------------------------------------------------------------------------------
// SaveProgramCounter сохраняет значение счетчика команд ЦПр в PSW процесса,
// который покидает процессор.
func (sch *Scheduler) SaveProgramCounter(index, pc int) {
	if index < 0 || index >= len(sch.Table) {
		return
	}
	sch.Table[index].PC = pc
}

// ----------------------------------------------------------------------------------
// RestoreProgramCounter возвращает значение PC из PSW выбранного процесса,
// которое затем загружается в аппаратный счетчик команд ЦПр.
func (sch *Scheduler) RestoreProgramCounter(index int) int {
	if index < 0 || index >= len(sch.Table) {
		return 0
	}
	return sch.Table[index].PC
}

// ----------------------------------------------------------------------------------
// Ядро планировщика (11 вариант)
// «Относительные и динамические приоритеты, приоритет для больших заданий растёт»

// GetNextProcess выбирает следующий процесс из списка готовности.
//
// Относительные приоритеты: работающий процесс не прерывается — выбор выполняется
// только при вызове планировщика (конец кванта или блокировка).
//
// Динамические приоритеты: перед выбором приоритет всех готовых процессов
// увеличивается. Шаг увеличения пропорционален размеру задания, поэтому у больших
// заданий приоритет растёт быстрее.
func (sch *Scheduler) GetNextProcess() int {
	bestIndex := -1
	bestPrior := -1

	var steps []string

	for i := range sch.Table {
		if sch.Table[i].State != process.StateReady {
			continue
		}

		// Шаг пропорционален размеру задания (не меньше единицы).
		step := sch.Table[i].Size / PriorityStepDivisor
		if step < 1 {
			step = 1
		}

		sch.Table[i].Prior += step
		steps = append(steps, fmt.Sprintf("P%d+%d", i, step))

		if sch.Table[i].Prior > bestPrior {
			bestPrior = sch.Table[i].Prior
			bestIndex = i
		}
	}

	if bestIndex == -1 {
		sch.Formula = "Нет готовых процессов — ЦПр простаивает (Ожидание)"
		return -1
	}

	sch.Formula = fmt.Sprintf(
		"P[i] += max(1, Size[i]/%d); выбран слот %d (ID %d), P=%d | %s",
		PriorityStepDivisor, bestIndex, sch.Table[bestIndex].ID, bestPrior,
		strings.Join(steps, " "),
	)
	return bestIndex
}

// ----------------------------------------------------------------------------------
// SelectNextProcess выбирает новый активный процесс, переводит его в состояние
// «Активен», сбрасывает его приоритет и устанавливает квант времени.
// Возвращает индекс выбранного слота или -1, если очередь готовности пуста.
func (sch *Scheduler) SelectNextProcess() int {
	next := sch.GetNextProcess()

	if next < 0 {
		sch.ActiveIndex = -1
		sch.QuantumLeft = 0
		return -1
	}

	sch.Table[next].State = process.StateActive
	sch.ActiveIndex = next

	// СБРОС ПРИОРИТЕТА: процесс получил ЦПр, его приоритет обнуляется.
	sch.Table[next].Prior = 0

	sch.QuantumLeft = sch.TimeQuantum
	if sch.QuantumLeft < 1 {
		sch.QuantumLeft = 1
	}

	return next
}
