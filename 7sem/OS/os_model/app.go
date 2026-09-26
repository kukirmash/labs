package main

import (
	"context"
	"os_model/model"

	"github.com/wailsapp/wails/v2/pkg/runtime"
)

type App struct {
	ctx     context.Context
	osModel *model.OSModel
}

func NewApp() *App {
	return &App{
		osModel: model.NewOSModel(),
	}
}

// startup вызывается при старте Wails приложения
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx

	// Привязываем функцию обновления интерфейса
	a.osModel.EmitUpdate = func(pc int, speed float64) {
		runtime.EventsEmit(a.ctx, "update_stats", pc, speed)
	}

	// Запускаем основной цикл модели в отдельной горутине (чтобы не блокировать UI)
	go a.osModel.Start()
}

// shutdown вызывается при закрытии приложения
func (a *App) shutdown(ctx context.Context) {
	a.osModel.Stop()
}

// Экспортируемые методы для вызова из JS (внешний цикл / директивы оператора)

func (a *App) IncreaseSpeed() {
	a.osModel.ChangeSpeed(true)
}

func (a *App) DecreaseSpeed() {
	a.osModel.ChangeSpeed(false)
}

func (a *App) QuitApp() {
	runtime.Quit(a.ctx)
}
