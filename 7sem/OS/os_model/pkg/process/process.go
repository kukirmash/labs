// Package process описывает задание (процесс) операционной системы:
// множество его состояний, слово состояния процесса (PSW) и генерацию
// последовательности команд для машины фон Неймана.
package process

import "math/rand"

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
	StateBlockIO   State = "Блокирован по обращению ко вводу (выводу)"
	StateSuspended State = "Приостановлен"
)

// ----------------------------------------------------------------------------------
// Параметры генерации команд процесса.
const (
	// MinTotalCommands, MaxTotalCommands — границы длительности задания
	// (числа команд до команды завершения).
	MinTotalCommands = 10
	MaxTotalCommands = 60

	// MinIOProbability, MaxIOProbability — границы вероятности того, что
	// очередная команда окажется обращением к устройству ввода-вывода.
	MinIOProbability = 0.10
	MaxIOProbability = 0.35

	// MaxIOBurst — максимальная длительность операции ввода-вывода (в тактах).
	MaxIOBurst = 5
)

// ----------------------------------------------------------------------------------
// Слово состояния процесса (PSW)
type PSW struct {
	ID    int   `json:"id"`
	Size  int   `json:"size"`
	PC    int   `json:"pc"`
	State State `json:"state"`
	Prior int   `json:"prior"`

	// TotalCommands — общее число команд задания: когда PC достигает этого
	// значения, генерируется команда завершения (OpEnd).
	TotalCommands int `json:"totalCommands"`
	// IOProbability — вероятность появления команды ввода-вывода
	// в очередном такте (0.0–1.0).
	IOProbability float64 `json:"ioProbability"`

	// LastResult — результат последней вычислительной операции (модель
	// регистра результата/ячейки аппаратной памяти).
	LastResult int `json:"lastResult"`
	// IORequestTicks — длительность запрошенной операции ввода-вывода
	// (заполняется АЛУ при выполнении команды OpIO).
	IORequestTicks int `json:"ioRequestTicks"`
}

// ----------------------------------------------------------------------------------
// IsPresent сообщает, что слот занят процессом (а не пуст).
func (psw PSW) IsPresent() bool {
	return psw.State != StateAbsent
}

// ----------------------------------------------------------------------------------
// New создаёт новое готовое задание с случайными параметрами длительности
// и вероятности обращения к вводу-выводу.
func New(id, size, prior int) PSW {
	psw := PSW{
		ID:             id,
		Size:           size,
		PC:             0,
		State:          StateReady,
		Prior:          prior,
		TotalCommands:  MinTotalCommands + rand.Intn(MaxTotalCommands-MinTotalCommands+1),
		IOProbability:  MinIOProbability + rand.Float64()*(MaxIOProbability-MinIOProbability),
		IORequestTicks: 0,
	}
	return psw
}

// ----------------------------------------------------------------------------------
// GenerateNextCommand имитирует чтение команды из памяти по адресу PC.
//
// Пока PC меньше TotalCommands, случайным образом генерируется либо
// вычислительная команда, либо команда обращения к устройству ввода-вывода
// (с вероятностью IOProbability). Как только PC достигает TotalCommands,
// возвращается команда завершения задания.
func (psw *PSW) GenerateNextCommand() Command {
	if psw.PC >= psw.TotalCommands {
		return Command{Code: OpEnd}
	}

	if rand.Float64() < psw.IOProbability {
		return Command{
			Code: OpIO,
			Arg1: 1 + rand.Intn(MaxIOBurst), // длительность операции IO в тактах
		}
	}

	return Command{
		Code: OpCompute,
		Arg1: rand.Intn(100),
		Arg2: rand.Intn(100),
	}
}
