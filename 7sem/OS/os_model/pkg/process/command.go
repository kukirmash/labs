package process

import "fmt"

// ----------------------------------------------------------------------------------
// OpCode — код операции команды процесса.
//
// Канонические определения типов команд живут в пакете process, так как
// команда является частью образа процесса, и именно PSW умеет её генерировать.
// Пакет cpu объявляет псевдонимы (cpu.Command, cpu.OpCompute, ...), чтобы
// не возникало циклической зависимости process → cpu.
type OpCode int

const (
	// OpCompute — вычислительная команда: двухместная операция над операндами,
	// выполняется АЛУ за один такт моделируемого времени.
	OpCompute OpCode = iota
	// OpIO — команда обращения к устройству ввода-вывода: выполняется
	// за N тактов (N хранится в Arg1).
	OpIO
	// OpEnd — команда завершения задания.
	OpEnd
)

// ToString возвращает человекочитаемое имя кода операции.
func (op OpCode) ToString() string {
	switch op {
	case OpCompute:
		return "Вычислительная"
	case OpIO:
		return "Ввод/вывод"
	case OpEnd:
		return "Завершение"
	default:
		return "Неизвестная"
	}
}

// ----------------------------------------------------------------------------------
// Command — команда процесса: код операции и два операнда (адреса).
type Command struct {
	Code OpCode `json:"code"` // код операции
	Arg1 int    `json:"arg1"` // первый операнд (для OpIO — длительность в тактах)
	Arg2 int    `json:"arg2"` // второй операнд
}

// ToString возвращает текстовое представление команды для подпрограммы индикации.
func (c Command) ToString() string {
	switch c.Code {
	case OpCompute:
		return fmt.Sprintf("%s: %d + %d", c.Code, c.Arg1, c.Arg2)
	case OpIO:
		return fmt.Sprintf("%s: %d такт(ов)", c.Code, c.Arg1)
	case OpEnd:
		return c.Code.ToString()
	default:
		return "Неизвестная команда"
	}
}
