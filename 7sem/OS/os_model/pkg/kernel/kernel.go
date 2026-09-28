// Package kernel — ядро модели операционной системы: оркестратор, который
// связывает процессор (машину фон Неймана), менеджер памяти, планировщик и
// подсистему ввода-вывода в единый цикл моделирования.
package kernel

import (
	"fmt"
	"math/rand"
	"sync"
	"time"

	"os_model/pkg/cpu"
	"os_model/pkg/io"
	"os_model/pkg/memory"
	"os_model/pkg/process"
	"os_model/pkg/scheduler"
)

// ----------------------------------------------------------------------------------
// MemorySize — объём оперативной памяти по умолчанию (в единицах памяти).
const MemorySize = 1000

// ----------------------------------------------------------------------------------
// SystemCallHandler — интерфейс системных вызовов ядра.
//
// Через него АЛУ процессора и подсистема ввода-вывода обращаются к ядру.
// Ядро (Model) реализует этот интерфейс: см. методы RequestIO, IOComplete
// и TerminateProcess ниже. Проверка реализуемости — var _ SystemCallHandler.
type SystemCallHandler interface {
	// RequestIO инициирует операцию ввода-вывода: процесс переводится
	// в состояние «Блокирован по обращению ко вводу (выводу)», устройство
	// получает его в обслуживание.
	RequestIO(psw *process.PSW)

	// IOComplete завершает операцию ввода-вывода и возвращает процесс
	// в список готовности.
	IOComplete(psw *process.PSW)

	// TerminateProcess завершает задание: освобождает слот таблицы процессов
	// и память, увеличивает счётчик выполненных заданий.
	TerminateProcess(pswID int)
}

// Компиляторная проверка: ядро реализует интерфейс системных вызовов.
var _ SystemCallHandler = (*Model)(nil)

// ----------------------------------------------------------------------------------
// Snapshot — согласованный снимок состояния всех подсистем модели,
// передаваемый подпрограмме индикации (EmitUpdate) для фронтенда.
type Snapshot struct {
	PC             int           `json:"pc"`             // аппаратный счётчик команд
	Speed          float64       `json:"speed"`          // тактовая частота
	Processes      []process.PSW `json:"processes"`      // копия таблицы процессов
	ActiveIndex    int           `json:"activeIndex"`    // номер активного слота
	CPUState       cpu.State     `json:"cpuState"`       // состояние процессора
	Formula        string        `json:"formula"`        // формула планировщика
	Command        cpu.Command   `json:"command"`        // последняя выбранная команда
	CommandText    string        `json:"commandText"`    // её текстовое описание
	Memory         memory.Stats  `json:"memory"`         // карта оперативной памяти
	IO             io.State      `json:"io"`             // состояние устройства IO
	CompletedTasks int           `json:"completedTasks"` // выполнено заданий
	TaskCounter    int           `json:"taskCounter"`    // сгенерировано заданий
	ResidentTasks  int           `json:"residentTasks"`  // резидентно в памяти
}

// ----------------------------------------------------------------------------------
// Model — модель операционной системы (ядро).
type Model struct {
	CPU       *cpu.CPU             // процессор (УУ + АЛУ)
	Memory    *memory.Manager      // менеджер оперативной памяти
	Scheduler *scheduler.Scheduler // планировщик (диспетчер задач)
	IO        *io.IOProcessor      // подсистема ввода-вывода

	TaskCounter    int         // счётчик сгенерированных заданий
	CompletedTasks int         // счётчик выполненных заданий
	NewTask        process.PSW // подготовленное к загрузке задание

	Running bool // признак работы основного цикла

	mu sync.Mutex // защита состояния модели

	// EmitUpdate — callback подпрограммы индикации: получает снимок состояния
	// всех подсистем и отправляет его на фронтенд.
	EmitUpdate func(Snapshot)
}

// ----------------------------------------------------------------------------------
// New создаёт ядро модели с параметрами по умолчанию.
func New() *Model {
	return &Model{
		CPU:       cpu.New(),
		Memory:    memory.New(MemorySize),
		Scheduler: scheduler.New(scheduler.DefaultSlots, scheduler.DefaultTimeQuantum),
		IO:        io.New(),
		NewTask:   process.PSW{State: process.StateAbsent},
	}
}

// ----------------------------------------------------------------------------------
// InitModel инициализирует модель: очищает таблицу процессов, память,
// устройство ввода-вывода и подготовку первого задания.
func (m *Model) InitModel() {
	m.TaskCounter = 0
	m.CompletedTasks = 0
	m.Scheduler.Reset()
	m.Memory.Reset()
	m.IO.Reset()
	m.CPU.Reset()
	m.NewTask = process.PSW{State: process.StateAbsent}
	m.GenerateTask()
}

// ----------------------------------------------------------------------------------
// GenerateTask генерирует новое задание со случайным размером и динамическим
// приоритетом: у больших заданий базовый приоритет выше.
func (m *Model) GenerateTask() {
	m.TaskCounter++

	size := rand.Intn(200) + 50 // Размер от 50 до 250 единиц
	basePrior := rand.Intn(5)   // Базовый приоритет

	// Динамический приоритет: для больших заданий увеличивается
	prior := basePrior + (size / scheduler.PriorityStepDivisor)

	m.NewTask = process.New(m.TaskCounter, size, prior)
}

// ----------------------------------------------------------------------------------
// CheckFreeMemory проверяет, достаточно ли свободной памяти для нового задания.
func (m *Model) CheckFreeMemory() bool {
	return m.Memory.FreeMemory() >= m.NewTask.Size
}

// ----------------------------------------------------------------------------------
// LoadTask загружает подготовленное задание: выделяет память, занимает слот
// в таблице процессов и сразу генерирует следующее задание.
func (m *Model) LoadTask() bool {
	if m.NewTask.State == process.StateAbsent {
		return false
	}
	if m.Scheduler.FindFreeSlot() < 0 {
		return false
	}
	if !m.Memory.Allocate(m.NewTask.Size, m.NewTask.ID) {
		return false
	}
	if !m.Scheduler.PlaceTask(m.NewTask) {
		m.Memory.Free(m.NewTask.ID) // откат выделенной памяти
		return false
	}

	m.GenerateTask() // Сразу генерируем следующее после успешной загрузки
	return true
}

// ----------------------------------------------------------------------------------
// LoadTasks загружает задания, пока хватает памяти и слотов в таблице.
func (m *Model) LoadTasks() {
	for m.LoadTask() {
		// Загружаем задания до исчерпания ресурсов
	}
}

// ----------------------------------------------------------------------------------
// Dispatch выбирает следующий процесс планировщиком и загружает его контекст
// в регистры процессора (в первую очередь — аппаратный счётчик команд).
func (m *Model) Dispatch() {
	idx := m.Scheduler.SelectNextProcess()
	if idx < 0 {
		m.CPU.State = cpu.CPUStateWait
		return
	}

	m.CPU.PC = m.Scheduler.RestoreProgramCounter(idx)
	m.CPU.State = cpu.CPUStateWork
}

// ----------------------------------------------------------------------------------
// AfterExecution обрабатывает исход такта активного процесса: блокировку,
// завершение или исчерпание кванта времени, и при необходимости назначает
// следующий процесс. Вызывается при удержанном m.mu.
func (m *Model) AfterExecution(psw *process.PSW, idx int) {
	terminated := psw.State == process.StateAbsent

	switch psw.State {
	case process.StateActive:
		// Счёт кванта времени: каждый такт уменьшаем остаток на единицу.
		m.Scheduler.QuantumLeft--
		if m.Scheduler.QuantumLeft <= 0 {
			m.Scheduler.SaveProgramCounter(idx, m.CPU.PC)
			psw.State = process.StateReady // возврат в очередь готовности
			m.Scheduler.ActiveIndex = -1
		}
	default:
		// Процесс заблокирован (IO/память) или завершён — ЦПр освобождается.
		m.Scheduler.ActiveIndex = -1
	}

	if m.Scheduler.ActiveIndex >= 0 {
		return
	}

	// Завершение задания освобождает память — загружаем новые задания (ЛР4).
	if terminated {
		m.LoadTasks()
	}

	m.CPU.State = cpu.CPUStateWait
	m.Dispatch()
}

// ----------------------------------------------------------------------------------
// Start запускает основной цикл моделирования.
//
// За один такт моделируемого времени выполняется:
//  1. пополнение очереди готовности (пока ЦПр простаивает);
//  2. диспетчеризация, если процессор свободен;
//  3. один такт процессора — CPU.Tick (выборка и выполнение команды);
//  4. один такт подсистемы ввода-вывода — IOProcessor.Tick;
//  5. формирование снимка всех подсистем и отправка его на фронтенд.
func (m *Model) Start() {
	m.mu.Lock()
	m.Running = true
	m.InitModel()
	m.LoadTasks() // Начальная загрузка заданий
	m.mu.Unlock()

	for {
		m.mu.Lock()
		if !m.Running {
			m.mu.Unlock()
			break
		}

		speed := m.CPU.Speed

		// 1. Пополнение очереди готовности, если процессор простаивает
		// (например, после завершения задания освободилась память).
		if m.Scheduler.ActiveIndex < 0 {
			m.LoadTasks()
		}

		// 2. Диспетчеризация: если ЦПр свободен — выбираем процесс.
		if m.Scheduler.ActiveIndex < 0 {
			m.Dispatch()
		}

		// 3. Один такт работы процессора.
		if idx := m.Scheduler.ActiveIndex; idx >= 0 {
			psw := &m.Scheduler.Table[idx]
			m.CPU.Tick(psw, m)
			m.AfterExecution(psw, idx)
		} else {
			m.CPU.State = cpu.CPUStateWait
		}

		// 4. Один такт работы подсистемы ввода-вывода.
		m.IO.Tick(m)

		// 5. Снимок состояния всех подсистем для индикации.
		snapshot := m.BuildSnapshot()
		m.mu.Unlock()

		if m.EmitUpdate != nil {
			m.EmitUpdate(snapshot)
		}

		delay := time.Duration(1000/speed) * time.Millisecond
		time.Sleep(delay)
	}
}

// ----------------------------------------------------------------------------------
// BuildSnapshot формирует согласованный снимок состояния модели.
// Вызывается при удержанном m.mu.
func (m *Model) BuildSnapshot() Snapshot {
	sc := m.Scheduler

	// Копия таблицы процессов для безопасной отправки на фронтенд
	procsCopy := make([]process.PSW, len(sc.Table))
	copy(procsCopy, sc.Table)

	result := 0
	if sc.ActiveIndex >= 0 && sc.ActiveIndex < len(sc.Table) {
		result = sc.Table[sc.ActiveIndex].LastResult
	}

	return Snapshot{
		PC:             m.CPU.PC,
		Speed:          m.CPU.Speed,
		Processes:      procsCopy,
		ActiveIndex:    sc.ActiveIndex,
		CPUState:       m.CPU.State,
		Formula:        sc.Formula,
		Command:        m.CPU.LastCommand,
		CommandText:    GetCommandStr(m.CPU.LastCommand, result, m.CPU.HasCommand),
		Memory:         m.Memory.GetStats(),
		IO:             m.IO.State(),
		CompletedTasks: m.CompletedTasks,
		TaskCounter:    m.TaskCounter,
		ResidentTasks:  sc.GetResidentCount(),
	}
}

// ----------------------------------------------------------------------------------
// GetCommandStr формирует описание выполняемой команды для подпрограммы
// индикации (требование практической части ЛР4, п. 3).
func GetCommandStr(cmd cpu.Command, result int, valid bool) string {
	if !valid {
		return "Нет команды"
	}

	switch cmd.Code {
	case cpu.OpCompute:
		return fmt.Sprintf("Вычислительная: %d + %d = %d", cmd.Arg1, cmd.Arg2, result)
	case cpu.OpIO:
		return fmt.Sprintf("Ввод/вывод: %d такт(ов)", cmd.Arg1)
	case cpu.OpEnd:
		return "Завершение задания"
	default:
		return "Неизвестная команда"
	}
}

// ----------------------------------------------------------------------------------
// Stop останавливает основной цикл моделирования.
func (m *Model) Stop() {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.Running = false
}

// ----------------------------------------------------------------------------------
// ChangeSpeed изменяет тактовую частоту процессора на 10%.
func (m *Model) ChangeSpeed(increase bool) {
	m.mu.Lock()
	defer m.mu.Unlock()

	delta := m.CPU.Speed * 0.10
	if increase {
		m.CPU.Speed += delta
	} else {
		m.CPU.Speed -= delta
	}

	if m.CPU.Speed > 1000.0 {
		m.CPU.Speed = 1000.0
	}
	if m.CPU.Speed < 0.1 {
		m.CPU.Speed = 0.1
	}
}

// ----------------------------------------------------------------------------------
// Реализация интерфейса SystemCallHandler.
//
// Методы вызываются из основного цикла моделирования (CPU.Tick / IO.Tick)
// при уже удержанном m.mu, поэтому сами блокировку не захватывают.

// RequestIO — системный вызов инициализации ввода-вывода: процесс сохраняет
// своё состояние (PSW уже актуален) и переводится в состояние «Блокирован
// по обращению ко вводу (выводу)», устройство начинает обслуживание.
func (m *Model) RequestIO(psw *process.PSW) {
	if psw == nil {
		return
	}

	psw.State = process.StateBlockIO
	m.IO.Start(psw)
}

// IOComplete — прерывание завершения ввода-вывода: ядро возвращает процесс
// в список готовности (его слот в таблице процессов сохраняется).
func (m *Model) IOComplete(psw *process.PSW) {
	if psw == nil {
		return
	}

	psw.State = process.StateReady
	psw.IORequestTicks = 0
}

// TerminateProcess — системный вызов завершения задания: удаление записи
// о процессе из таблицы, освобождение статической и динамической памяти,
// учёт выполненных заданий.
func (m *Model) TerminateProcess(pswID int) {
	idx := m.Scheduler.FindProcess(pswID)
	if idx < 0 {
		return
	}

	m.Scheduler.ClearSlot(idx) // освобождение записи в таблице процессов
	m.Memory.Free(pswID)       // освобождение памяти задания
	m.CompletedTasks++         // увеличиваем число выполненных заданий
}
