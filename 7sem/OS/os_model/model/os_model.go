package model

import (
	"math/rand"
	"sync"
	"time"
)

// ----------------------------------------------------------------------------------
// 3. Описание типа для состояния процесса
type ProcessState string

const (
	StateAbsent  ProcessState = "Отсутствует"
	StateReady   ProcessState = "Готов"
	StateRunning ProcessState = "Выполняется"
)

// ----------------------------------------------------------------------------------
// 4. Описание слова состояния процесса (PSW)
type PSW struct {
	ID    int          `json:"id"`
	Size  int          `json:"size"`
	PC    int          `json:"pc"`
	State ProcessState `json:"state"`
	Prior int          `json:"prior"`
}

// ----------------------------------------------------------------------------------
type OSModel struct {
	// 1. Параметры процессора
	PC    int
	Speed float64

	// 2. Параметры системы
	MemSize     int
	TaskCounter int

	// 5, 6. Массив слов состояний и переменная для нового задания
	Processes [16]PSW
	NewTask   PSW

	Running    bool
	mu         sync.Mutex
	EmitUpdate func(pc int, speed float64, procs []PSW) // Обновленная сигнатура для отправки массива
}

// ----------------------------------------------------------------------------------
func NewOSModel() *OSModel {
	return &OSModel{
		PC:      0,
		Speed:   1.0,
		MemSize: 1000, // Значение памяти по умолчанию
		Running: false,
	}
}

// ----------------------------------------------------------------------------------
// 7. Инициализация массива слов состояний
func (m *OSModel) InitModel() {
	m.TaskCounter = 0
	for i := range m.Processes {
		m.Processes[i] = PSW{State: StateAbsent}
	}
	m.GenerateTask() // Подготовка первого задания
}

// ----------------------------------------------------------------------------------
// 8. Генерация нового задания (со значениями по умолчанию и динамическим приоритетом)
func (m *OSModel) GenerateTask() {
	m.TaskCounter++
	size := rand.Intn(200) + 50 // Размер от 50 до 250 единиц
	basePrior := rand.Intn(5)   // Базовый приоритет

	// Динамический приоритет: для больших заданий увеличивается
	prior := basePrior + (size / 50)

	m.NewTask = PSW{
		ID:    m.TaskCounter,
		Size:  size,
		PC:    0,
		State: StateReady,
		Prior: prior,
	}
}

// ----------------------------------------------------------------------------------
// 9. Проверка наличия доступной памяти (формула вычисления)
func (m *OSModel) CheckFreeMemory() bool {
	usedMemory := 0
	for _, p := range m.Processes {
		if p.State != StateAbsent {
			usedMemory += p.Size
		}
	}
	freeMemory := m.MemSize - usedMemory
	return freeMemory >= m.NewTask.Size
}

// ----------------------------------------------------------------------------------
// 10. Загрузка нового задания
func (m *OSModel) LoadTask() bool {
	if !m.CheckFreeMemory() {
		return false
	}

	// Поиск свободного места в таблице процессов
	for i := range m.Processes {
		if m.Processes[i].State == StateAbsent {
			m.Processes[i] = m.NewTask
			m.GenerateTask() // Сразу генерируем следующее после успешной загрузки
			return true
		}
	}
	return false
}

// ----------------------------------------------------------------------------------
// Start запускает основной цикл
func (m *OSModel) Start() {
	m.mu.Lock()
	m.Running = true
	m.mu.Unlock()

	m.InitModel()

	// 11. Цикл загрузки заданий для начала моделирования
	for m.LoadTask() {
		// Загружаем задания, пока хватает памяти и места в массиве
	}

	for {
		m.mu.Lock()
		if !m.Running {
			m.mu.Unlock()
			break
		}

		speed := m.Speed
		m.PC++
		pc := m.PC

		// Копия массива процессов для безопасной отправки на фронтенд
		var procsCopy []PSW
		for _, p := range m.Processes {
			procsCopy = append(procsCopy, p)
		}
		m.mu.Unlock()

		// 12. Индикация (отправка обновленных данных на фронтенд)
		if m.EmitUpdate != nil {
			m.EmitUpdate(pc, speed, procsCopy)
		}

		delay := time.Duration(1000/speed) * time.Millisecond
		time.Sleep(delay)
	}
}

// ----------------------------------------------------------------------------------
func (m *OSModel) Stop() {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.Running = false
}

// ----------------------------------------------------------------------------------
func (m *OSModel) ChangeSpeed(increase bool) {
	m.mu.Lock()
	defer m.mu.Unlock()
	delta := m.Speed * 0.10
	if increase {
		m.Speed += delta
	} else {
		m.Speed -= delta
	}
	if m.Speed > 1000.0 {
		m.Speed = 1000.0
	}
	if m.Speed < 0.1 {
		m.Speed = 0.1
	}
}

//----------------------------------------------------------------------------------
