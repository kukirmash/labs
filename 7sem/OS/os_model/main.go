// Точка входа Wails-приложения.
//
// Wails v2 собирает пакет, расположенный в корне проекта, а директива
// //go:embed не умеет обращаться к путям через «..», поэтому в корне остаётся
// минимальный загрузчик ассетов. Сборка зависимостей и запуск приложения
// находятся в cmd/main.go (композиционный корень).
package main

import (
	"embed"

	"os_model/cmd"
)

//go:embed all:frontend/dist
var assets embed.FS

func main() {
	cmd.Run(assets)
}
