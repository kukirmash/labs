// Package process описывает задание (процесс) операционной системы:
// множество его состояний и слово состояния процесса (PSW).
package process

// ----------------------------------------------------------------------------------
// Состояния процесса
type State string

const (
	StateAbsent    State = "Отсутствует"
	StateReady     State = "Готов"
	StateLoading   State = "Загружается"
	StateActive    State = "Активен"
	StateInitIO    State = "Инициализация ввода вывода"
	StateEndIO     State = "Конец ввода (вывода)"
	StateBlockMem  State = "Блокирован по обращению к памяти"
	StateBlockIO   State = "Блокирован по выполнению ввода (вывода)"
	StateSuspended State = "Приостановлен"
)

// ----------------------------------------------------------------------------------
// Слово состояния процесса (PSW)
type PSW struct {
	ID    int   `json:"id"`
	Size  int   `json:"size"`
	PC    int   `json:"pc"`
	State State `json:"state"`
	Prior int   `json:"prior"`
}

// ----------------------------------------------------------------------------------
// Present сообщает, что слот занят процессом (а не пуст).
func (p PSW) Present() bool {
	return p.State != StateAbsent
}

// ----------------------------------------------------------------------------------
func New(id, size, prior int) PSW {
	psw := PSW{
		ID:    id,
		Size:  size,
		PC:    0,
		State: StateReady,
		Prior: prior,
	}
	return psw
}

// ----------------------------------------------------------------------------------
