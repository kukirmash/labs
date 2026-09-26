package model

import (
	"sync"
	"time"
)

// OSModel представляет ядро операционной системы
type OSModel struct {
	PC          int
	Speed       float64
	Running     bool
	mu          sync.Mutex
	EmitUpdate  func(pc int, speed float64) // Функция обратного вызова для обновления UI
}

func NewOSModel() *OSModel {
	return &OSModel{
		PC:      0,
		Speed:   1.0, // Начальная скорость: 1 такт в секунду
		Running: false,
	}
}

// Заглушки для будущих лабораторных
func (m *OSModel) InitModel() {}
func (m *OSModel) GenerateTask() {}
func (m *OSModel) CheckFreeMemory() bool { return true }
func (m *OSModel) LoadTask() {}

// Start запускает "внутренний цикл" моделирования
func (m *OSModel) Start() {
	m.mu.Lock()
	m.Running = true
	m.mu.Unlock()

	// Вызов заглушек
	m.InitModel()
	m.GenerateTask()
	m.CheckFreeMemory()
	m.LoadTask()

	for {
		m.mu.Lock()
		if !m.Running {
			m.mu.Unlock()
			break
		}
		
		speed := m.Speed
		m.PC++
		pc := m.PC
		m.mu.Unlock()

		// Индикация параметров (отправка на фронтенд)
		if m.EmitUpdate != nil {
			m.EmitUpdate(pc, speed)
		}

		// Программная задержка
		// Если скорость 2 такта/сек, задержка будет 500 мс (1000/2)
		delay := time.Duration(1000/speed) * time.Millisecond
		time.Sleep(delay)
	}
}

// Stop останавливает моделирование
func (m *OSModel) Stop() {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.Running = false
}

// ChangeSpeed изменяет скорость на 5-10% (в данном случае на 10%)
func (m *OSModel) ChangeSpeed(increase bool) {
	m.mu.Lock()
	defer m.mu.Unlock()
	
	delta := m.Speed * 0.10
	if increase {
		m.Speed += delta
	} else {
		m.Speed -= delta
	}
	
	// Ограничения
	if m.Speed > 1000.0 {
		m.Speed = 1000.0
	}
	if m.Speed < 0.1 {
		m.Speed = 0.1
	}
}