package cpu

import "os_model/pkg/process"

// ----------------------------------------------------------------------------------
// Псевдонимы типов команд.
//
// Канонические определения Command и OpCode находятся в пакете process
// (см. pkg/process/command.go): команда — часть образа процесса, и именно PSW
// генерирует очередную команду. Псевдонимы позволяют использовать привычные
// имена cpu.Command, cpu.OpCompute, ... и не создают циклической зависимости
// process → cpu.
type OpCode = process.OpCode
type Command = process.Command

const (
	// OpCompute — вычислительная команда (1 такт).
	OpCompute = process.OpCompute
	// OpIO — команда обращения к устройству ввода-вывода (N тактов).
	OpIO = process.OpIO
	// OpEnd — команда завершения задания.
	OpEnd = process.OpEnd
)
