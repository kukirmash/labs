package main

import (
	"embed"
	"fmt"
	"os"

	"github.com/wailsapp/wails/v2"
	"github.com/wailsapp/wails/v2/pkg/options"
	"github.com/wailsapp/wails/v2/pkg/options/assetserver"
)

//go:embed all:frontend/dist
var assets embed.FS

func main() {
	// Выдача справки по запросу из командной строки
	if len(os.Args) > 1 && os.Args[1] == "/?" {
		fmt.Println("=== Модель ОС. Лабораторная работа №1 ===")
		fmt.Println("Управление:")
		fmt.Println("  [+] - Увеличить скорость на 10%")
		fmt.Println("  [-] - Уменьшить скорость на 10%")
		fmt.Println("  [ESC] - Выход из программы")
		return
	}

	app := NewApp()

	err := wails.Run(&options.App{
		Title:  "Модель ОС",
		Width:  800,
		Height: 600,
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