package cmd

import (
	"context"

	"os_model/pkg/kernel"

	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// ----------------------------------------------------------------------------------
// App — адаптер между графическим интерфейсом (Wails) и ядром модели.
type App struct {
	ctx   context.Context
	model *kernel.Model
}

// ----------------------------------------------------------------------------------
// startup вызывается при старте Wails-приложения: подписывает ядро на отправку
// снимков состояния на фронтенд и запускает основной цикл моделирования.
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx

	// Привязываем подпрограмму индикации: ядро передаёт снимок всех подсистем
	// (процессор, процессы, память, устройство ввода-вывода, команда).
	a.model.EmitUpdate = func(snapshot kernel.Snapshot) {
		runtime.EventsEmit(a.ctx, "update_stats", snapshot)
	}

	go a.model.Start()
}

// ----------------------------------------------------------------------------------
// shutdown вызывается при закрытии приложения.
func (a *App) shutdown(ctx context.Context) {
	a.model.Stop()
}

// ----------------------------------------------------------------------------------
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
