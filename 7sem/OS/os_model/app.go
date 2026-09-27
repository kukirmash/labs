package main

import (
	"context"

	"os_model/os_model"
	"os_model/process"

	"github.com/wailsapp/wails/v2/pkg/runtime"
)

type App struct {
	ctx   context.Context
	model *os_model.Model
}

func NewApp() *App {
	return &App{
		model: os_model.New(),
	}
}

// startup вызывается при старте Wails приложения
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx

	// Привязываем функцию обновления интерфейса: таблица процессов, активный
	// процесс, состояние ЦПр и строковое представление формулы планировщика.
	a.model.EmitUpdate = func(pc int, speed float64, procs []process.PSW, activeIndex int, cpuState os_model.CPUState, formula string) {
		runtime.EventsEmit(a.ctx, "update_stats", pc, speed, procs, activeIndex, cpuState, formula)
	}

	go a.model.Start()
}

// shutdown вызывается при закрытии приложения
func (a *App) shutdown(ctx context.Context) {
	a.model.Stop()
}

// Экспортируемые методы для вызова из JS (внешний цикл / директивы оператора)

func (a *App) IncreaseSpeed() {
	a.model.ChangeSpeed(true)
}

func (a *App) DecreaseSpeed() {
	a.model.ChangeSpeed(false)
}

func (a *App) QuitApp() {
	runtime.Quit(a.ctx)
}
