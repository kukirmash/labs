// Package os_model описывает модель операционной системы: параметры процессора
// и памяти, генерацию и загрузку заданий, а также основной цикл моделирования.
package os_model

import (
	"math/rand"
	"sync"
	"time"

	"os_model/process"
	"os_model/scheduler"
)

// ----------------------------------------------------------------------------------
// CPUState — состояние центрального процессора.
type CPUState string

const (
	CPUStateWork CPUState = "Работа"
	CPUStateWait CPUState = "Ожидание"
)

// CPU — центральный процессор.
type CPU struct {
	PC    int      // аппаратный счетчик команд
	Speed float64  // тактовая частота (тактов/сек)
	State CPUState // состояние: «Работа» или «Ожидание»
}

// Memory — параметры оперативной памяти.
type Memory struct {
	Size int // объем памяти
}

// ----------------------------------------------------------------------------------
// Model — модель операционной системы.
type Model struct {
	CPU    CPU
	Memory Memory

	TaskCounter int
	NewTask     process.PSW

	Running bool

	// Планировщик (диспетчер задач)
	Scheduler *scheduler.Scheduler

	mu sync.Mutex
	// Обновленная сигнатура: массив процессов, активный процесс, состояние ЦПр и формула
	EmitUpdate func(pc int, speed float64, procs []process.PSW, activeIndex int, cpuState CPUState, formula string)
}

// ----------------------------------------------------------------------------------
func New() *Model {
	return &Model{
		CPU:       CPU{PC: 0, Speed: 1.0, State: CPUStateWait},
		Memory:    Memory{Size: 1000}, // Значение памяти по умолчанию
		Running:   false,
		Scheduler: scheduler.New(scheduler.DefaultSlots, scheduler.DefaultTimeQuantum),
		NewTask:   process.PSW{State: process.StateAbsent},
	}
}

// ----------------------------------------------------------------------------------
// Инициализация модели: очистка таблицы процессов и подготовка первого задания.
func (m *Model) InitModel() {
	m.TaskCounter = 0
	m.Scheduler.Reset()
	m.GenerateTask()
}

// ----------------------------------------------------------------------------------
// Генерация нового задания (со значениями по умолчанию и динамическим приоритетом)
func (m *Model) GenerateTask() {
	m.TaskCounter++
	size := rand.Intn(200) + 50 // Размер от 50 до 250 единиц
	basePrior := rand.Intn(5)   // Базовый приоритет

	// Динамический приоритет: для больших заданий увеличивается
	prior := basePrior + (size / scheduler.PriorityStepDivisor)

	m.NewTask = process.New(m.TaskCounter, size, prior)
}

// ----------------------------------------------------------------------------------
// Проверка наличия доступной памяти (формула вычисления)
func (m *Model) CheckFreeMemory() bool {
	freeMemory := m.Memory.Size - m.Scheduler.UsedMemory()
	return freeMemory >= m.NewTask.Size
}

// ----------------------------------------------------------------------------------
// Загрузка нового задания
func (m *Model) LoadTask() bool {
	if !m.CheckFreeMemory() {
		return false
	}
	if !m.Scheduler.PlaceTask(m.NewTask) {
		return false
	}
	m.GenerateTask() // Сразу генерируем следующее после успешной загрузки
	return true
}

// ----------------------------------------------------------------------------------
// Start запускает основной цикл моделирования
func (m *Model) Start() {
	m.mu.Lock()
	m.Running = true
	m.mu.Unlock()

	m.InitModel()

	// Цикл загрузки заданий для начала моделирования
	for m.LoadTask() {
		// Загружаем задания, пока хватает памяти и места в массиве
	}

	for {
		m.mu.Lock()
		if !m.Running {
			m.mu.Unlock()
			break
		}

		speed := m.CPU.Speed
		sc := m.Scheduler

		// Если процессор свободен — сразу запускаем планировщик.
		if sc.ActiveIndex < 0 {
			m.CPU.PC = sc.SelectNextProcess(m.CPU.PC)
		}

		if sc.ActiveIndex >= 0 {
			// Выполнение одной команды активного процесса.
			m.CPU.PC++
			sc.Table[sc.ActiveIndex].PC = m.CPU.PC // актуальный PC для индикации

			// Счёт кванта времени: каждый такт уменьшаем остаток на единицу.
			sc.QuantumLeft--

			// Квант исчерпан — вызываем планировщик.
			if sc.QuantumLeft <= 0 {
				sc.SaveProcessState(sc.ActiveIndex, m.CPU.PC)
				// Активный процесс возвращается в очередь готовности.
				sc.Table[sc.ActiveIndex].State = process.StateReady
				sc.ActiveIndex = -1

				// Выбор следующего процесса (или простой ЦПр).
				m.CPU.PC = sc.SelectNextProcess(m.CPU.PC)
			}
		}

		// ЦПр работает, если планировщик выбрал процесс, иначе ожидает.
		m.UpdateCPUState()

		pc := m.CPU.PC
		activeIndex := sc.ActiveIndex
		cpuState := m.CPU.State
		formula := sc.Formula

		// Копия таблицы процессов для безопасной отправки на фронтенд
		procsCopy := make([]process.PSW, len(sc.Table))
		copy(procsCopy, sc.Table)
		m.mu.Unlock()

		// Индикация (отправка обновленных данных на фронтенд)
		if m.EmitUpdate != nil {
			m.EmitUpdate(pc, speed, procsCopy, activeIndex, cpuState, formula)
		}

		delay := time.Duration(1000/speed) * time.Millisecond
		time.Sleep(delay)
	}
}

// ----------------------------------------------------------------------------------
// UpdateCPUState устанавливает состояние ЦПр: «Работа», если планировщик выбрал
// активный процесс, и «Ожидание», если очередь готовности пуста.
func (m *Model) UpdateCPUState() {
	if m.Scheduler != nil && m.Scheduler.ActiveIndex >= 0 {
		m.CPU.State = CPUStateWork
	} else {
		m.CPU.State = CPUStateWait
	}
}

// ----------------------------------------------------------------------------------
func (m *Model) Stop() {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.Running = false
}

// ----------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------
