// Package memory реализует менеджер оперативной памяти: распределение
// непрерывных участков (блоков) между процессами и учёт свободной памяти.
package memory

import (
	"sort"
	"sync"
)

// ----------------------------------------------------------------------------------
// Block — непрерывный участок оперативной памяти [Start, Start+Size).
type Block struct {
	Start int `json:"start"` // начальный адрес блока
	Size  int `json:"size"`  // размер блока в единицах памяти
}

// End возвращает адрес, следующий за последним байтом блока (граница справа).
func (b Block) End() int {
	return b.Start + b.Size
}

// Allocation — занятый участок памяти с указанием владельца (ID процесса) —
// используется подпрограммой индикации.
type Allocation struct {
	ProcessID int `json:"processId"`
	Start     int `json:"start"`
	Size      int `json:"size"`
}

// Stats — снимок карты памяти для передачи на фронтенд.
type Stats struct {
	Total      int          `json:"total"`      // всего памяти
	Used       int          `json:"used"`       // занято
	Free       int          `json:"free"`       // свободно
	Fragments  int          `json:"fragments"`  // число свободных фрагментов
	UsedBlocks []Allocation `json:"usedBlocks"` // занятые блоки (по возрастанию адреса)
	FreeBlocks []Block      `json:"freeBlocks"` // свободные блоки (по возрастанию адреса)
}

// ----------------------------------------------------------------------------------
// Manager — менеджер оперативной памяти.
//
// Занятая память хранится как map[ID процесса]Block, свободная — как
// упорядоченный по адресам список блоков. При освобождении соседние
// свободные участки склеиваются в один (борьба с фрагментацией).
type Manager struct {
	total int           // общий объём памяти
	used  map[int]Block // занятые блоки: ID процесса → блок
	free  []Block       // свободные блоки, упорядочены по Start
	mu    sync.Mutex    // защита состояния менеджера
}

// ----------------------------------------------------------------------------------
// New создаёт менеджер памяти заданного объёма. Изначально вся память свободна.
func New(total int) *Manager {
	if total < 0 {
		total = 0
	}
	m := &Manager{
		total: total,
		used:  make(map[int]Block),
	}
	if total > 0 {
		m.free = []Block{{Start: 0, Size: total}}
	}
	return m
}

// ----------------------------------------------------------------------------------
// GetTotal возвращает общий объём памяти.
func (m *Manager) GetTotal() int {
	m.mu.Lock()
	defer m.mu.Unlock()
	return m.total
}

// ----------------------------------------------------------------------------------
// Allocate выделяет процессу processID непрерывный блок размера size
// (первый подходящий — стратегия first fit). Возвращает false, если
// подходящего свободного блока нет или процесс уже владеет памятью.
func (m *Manager) Allocate(size, processID int) bool {
	if size <= 0 {
		return false
	}

	m.mu.Lock()
	defer m.mu.Unlock()

	if _, exists := m.used[processID]; exists {
		return false
	}

	for i, b := range m.free {
		if b.Size < size {
			continue
		}

		allocated := Block{Start: b.Start, Size: size}
		if b.Size == size {
			// Блок расходуется целиком — удаляем его из списка свободных.
			m.free = append(m.free[:i], m.free[i+1:]...)
		} else {
			// Отрезаем часть блока, остаток остаётся свободным.
			m.free[i].Start += size
			m.free[i].Size -= size
		}

		m.used[processID] = allocated
		return true
	}

	return false
}

// ----------------------------------------------------------------------------------
// Free освобождает память, принадлежащую процессу processID, и склеивает
// освободившийся участок с соседними свободными блоками.
func (m *Manager) Free(processID int) {
	m.mu.Lock()
	defer m.mu.Unlock()

	block, ok := m.used[processID]
	if !ok {
		return
	}
	delete(m.used, processID)
	m.Release(block)
}

// ----------------------------------------------------------------------------------
// Release вставляет блок в список свободных, сохраняя порядок по адресам,
// и выполняет склейку с предыдущим и следующим блоками.
func (m *Manager) Release(b Block) {
	// Двоичный поиск позиции для вставки (список отсортирован по Start).
	i := sort.Search(len(m.free), func(i int) bool {
		return m.free[i].Start >= b.Start
	})

	m.free = append(m.free, Block{})
	copy(m.free[i+1:], m.free[i:])
	m.free[i] = b

	// Склейка с предыдущим блоком, если они соприкасаются.
	if i > 0 && m.free[i-1].End() == m.free[i].Start {
		m.free[i-1].Size += m.free[i].Size
		m.free = append(m.free[:i], m.free[i+1:]...)
		i--
	}

	// Склейка со следующим блоком, если они соприкасаются.
	if i+1 < len(m.free) && m.free[i].End() == m.free[i+1].Start {
		m.free[i].Size += m.free[i+1].Size
		m.free = append(m.free[:i+1], m.free[i+2:]...)
	}
}

// ----------------------------------------------------------------------------------
// GetUsed возвращает суммарный объём занятой памяти.
func (m *Manager) GetUsed() int {
	m.mu.Lock()
	defer m.mu.Unlock()

	used := 0
	for _, b := range m.used {
		used += b.Size
	}
	return used
}

// ----------------------------------------------------------------------------------
// FreeMemory возвращает суммарный объём свободной памяти.
func (m *Manager) FreeMemory() int {
	m.mu.Lock()
	defer m.mu.Unlock()

	free := 0
	for _, b := range m.free {
		free += b.Size
	}
	return free
}

// ----------------------------------------------------------------------------------
// GetAllocations возвращает занятые блоки, упорядоченные по адресу.
func (m *Manager) GetAllocations() []Allocation {
	m.mu.Lock()
	defer m.mu.Unlock()

	list := make([]Allocation, 0, len(m.used))
	for id, b := range m.used {
		list = append(list, Allocation{ProcessID: id, Start: b.Start, Size: b.Size})
	}
	sort.Slice(list, func(i, j int) bool {
		return list[i].Start < list[j].Start
	})
	return list
}

// ----------------------------------------------------------------------------------
// GetFreeBlocks возвращает копию списка свободных блоков (по возрастанию адреса).
func (m *Manager) GetFreeBlocks() []Block {
	m.mu.Lock()
	defer m.mu.Unlock()

	blocks := make([]Block, len(m.free))
	copy(blocks, m.free)
	return blocks
}

// ----------------------------------------------------------------------------------
// GetStats формирует согласованный снимок карты памяти для индикации.
func (m *Manager) GetStats() Stats {
	m.mu.Lock()
	defer m.mu.Unlock()

	stats := Stats{
		Total:      m.total,
		UsedBlocks: make([]Allocation, 0, len(m.used)),
		FreeBlocks: make([]Block, len(m.free)),
	}

	for id, b := range m.used {
		stats.Used += b.Size
		stats.UsedBlocks = append(stats.UsedBlocks, Allocation{ProcessID: id, Start: b.Start, Size: b.Size})
	}
	sort.Slice(stats.UsedBlocks, func(i, j int) bool {
		return stats.UsedBlocks[i].Start < stats.UsedBlocks[j].Start
	})

	for i, b := range m.free {
		stats.Free += b.Size
		stats.FreeBlocks[i] = b
	}
	stats.Fragments = len(m.free)

	return stats
}

// ----------------------------------------------------------------------------------
// Reset возвращает менеджер в исходное состояние: вся память свободна.
func (m *Manager) Reset() {
	m.mu.Lock()
	defer m.mu.Unlock()

	m.used = make(map[int]Block)
	m.free = nil
	if m.total > 0 {
		m.free = []Block{{Start: 0, Size: m.total}}
	}
}
