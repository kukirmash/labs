// Package io реализует подсистему ввода-вывода: модель устройства,
// выполняющего операцию заданное число тактов, и её взаимодействие с ядром
// через системные вызовы.
package io

import "os_model/pkg/process"

// ----------------------------------------------------------------------------------
// DefaultDuration — длительность операции ввода-вывода по умолчанию
// (используется, если команда не задала своё значение).
const DefaultDuration = 5

// ----------------------------------------------------------------------------------
// SystemCallHandler — интерфейс системных вызовов, которые подсистема
// ввода-вывода адресует ядру. Объявлен на стороне потребителя, чтобы пакет io
// не зависел от пакета kernel.
type SystemCallHandler interface {
	// IOComplete вызывается по завершении операции ввода-вывода: ядро
	// возвращает процесс в состояние «Готов».
	IOComplete(psw *process.PSW)
}

// ----------------------------------------------------------------------------------
// State — снимок состояния устройства ввода-вывода для индикации.
type State struct {
	Busy       bool `json:"busy"`       // устройство занято выполнением операции
	ProcessID  int  `json:"processId"`  // ID обслуживаемого процесса (0 — нет)
	TicksLeft  int  `json:"ticksLeft"`  // сколько тактов осталось
	TotalTicks int  `json:"totalTicks"` // полная длительность операции
}

// ----------------------------------------------------------------------------------
// IOProcessor — модель процессора (устройства) ввода-вывода.
type IOProcessor struct {
	// ActiveProcess — процесс, ожидающий завершения операции ввода-вывода.
	ActiveProcess *process.PSW
	// TicksLeft — сколько тактов осталось до завершения операции.
	TicksLeft int
	// TotalTicks — полная длительность текущей операции (для индикации).
	TotalTicks int
}

// ----------------------------------------------------------------------------------
// New создаёт свободное устройство ввода-вывода.
func New() *IOProcessor {
	return &IOProcessor{}
}

// ----------------------------------------------------------------------------------
// Start инициирует операцию ввода-вывода для процесса. Длительность берётся из
// PSW (её заполняет АЛУ при выполнении команды OpIO), по умолчанию DefaultDuration.
func (p *IOProcessor) Start(psw *process.PSW) {
	if psw == nil {
		return
	}

	duration := psw.IORequestTicks
	if duration <= 0 {
		duration = DefaultDuration
	}

	p.ActiveProcess = psw
	p.TicksLeft = duration
	p.TotalTicks = duration
}

// ----------------------------------------------------------------------------------
// Tick выполняет один такт работы устройства: уменьшает счётчик оставшихся
// тактов и по его достижении нуля инициирует прерывание завершения
// ввода-вывода через системный вызов ядра sys.IOComplete.
func (p *IOProcessor) Tick(sys SystemCallHandler) {
	if p.ActiveProcess == nil {
		return
	}

	p.TicksLeft--
	if p.TicksLeft > 0 {
		return
	}

	// Операция завершена: устройство освобождается, ядро получает прерывание.
	done := p.ActiveProcess
	p.ActiveProcess = nil
	p.TicksLeft = 0
	p.TotalTicks = 0

	if sys != nil {
		sys.IOComplete(done)
	}
}

// ----------------------------------------------------------------------------------
// Busy сообщает, занято ли устройство выполнением операции.
func (p *IOProcessor) Busy() bool {
	return p.ActiveProcess != nil
}

// ----------------------------------------------------------------------------------
// State формирует снимок состояния устройства для индикации.
func (p *IOProcessor) State() State {
	state := State{
		Busy:       p.ActiveProcess != nil,
		TicksLeft:  p.TicksLeft,
		TotalTicks: p.TotalTicks,
	}
	if p.ActiveProcess != nil {
		state.ProcessID = p.ActiveProcess.ID
	}
	return state
}

// ----------------------------------------------------------------------------------
// Reset освобождает устройство.
func (p *IOProcessor) Reset() {
	p.ActiveProcess = nil
	p.TicksLeft = 0
	p.TotalTicks = 0
}
