// Package cmd — композиционный корень приложения: здесь собираются зависимости
// (ядро модели, адаптер Wails) и запускается графическое приложение.
package cmd

import (
	"fmt"
	"io/fs"
	"os"

	"os_model/pkg/kernel"

	"github.com/wailsapp/wails/v2"
	"github.com/wailsapp/wails/v2/pkg/options"
	"github.com/wailsapp/wails/v2/pkg/options/assetserver"
)

// ----------------------------------------------------------------------------------
// NewApp собирает приложение: создаёт ядро модели и адаптер для Wails.
func NewApp() *App {
	return &App{
		model: kernel.New(),
	}
}

// ----------------------------------------------------------------------------------
// Run запускает приложение Wails с переданными ассетами фронтенда.
// Ассеты встраиваются в корневом пакете main (требование Wails + go:embed).
func Run(assets fs.FS) {
	// Выдача справки по запросу из командной строки
	if len(os.Args) > 1 && os.Args[1] == "/?" {
		printHelp()
		return
	}

	app := NewApp()

	err := wails.Run(&options.App{
		Title:  "Модель ОС — Лабораторная работа №4",
		Width:  1180,
		Height: 820,
		AssetServer: &assetserver.Options{
			Assets: assets,
		},
		OnStartup:  app.startup,
		OnShutdown: app.shutdown,
		Bind: []interface{}{
			app,
		},
	})

	if err != nil {
		fmt.Println("Error:", err.Error())
	}
}

// ----------------------------------------------------------------------------------
// printHelp выводит справку по управлению моделью.
func printHelp() {
	fmt.Println("=== Модель ОС. Лабораторная работа №4 (выполнение команд процесса) ===")
	fmt.Println("Управление:")
	fmt.Println("  [+] - Увеличить скорость на 10%")
	fmt.Println("  [-] - Уменьшить скорость на 10%")
	fmt.Println("  [ESC] - Выход из программы")
}
