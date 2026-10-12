package main

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
)

// IP — таблица начальной перестановки (Initial Permutation) с нумерацией битов от 1 до 64
var IP = [64]int{
	58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6,
	64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17, 9, 1,
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5,
	63, 55, 47, 39, 31, 23, 15, 7,
}

// PKCS7Pad дополняет срез байт до кратности blockSize (для DES это 8 байт / 64 бита)
func PKCS7Pad(data []byte, blockSize int) []byte {
	//
	padding := blockSize - (len(data) % blockSize)

	padBytes := bytes.Repeat([]byte{byte(padding)}, padding)
	return append(data, padBytes...)
}

// PKCS7Unpad удаляет дополнение PKCS#7 после дешифрования
func PKCS7Unpad(data []byte) ([]byte, error) {
	length := len(data)
	if length == 0 || length%8 != 0 {
		return nil, errors.New("некорректная длина данных для снятия дополнения")
	}

	padLen := int(data[length-1])
	if padLen == 0 || padLen > 8 {
		return nil, errors.New("некорректное значение байта дополнения")
	}

	for i := length - padLen; i < length; i++ {
		if data[i] != byte(padLen) {
			return nil, errors.New("нарушена целостность байтов дополнения")
		}
	}

	return data[:length-padLen], nil
}

// StringToBlocks преобразует исходную строку в срез 64-битных блоков (uint64)
func StringToBlocks(s string) []uint64 {
	// 1. Получаем байты строки (в кодировке UTF-8 или ASCII)
	data := []byte(s)
	fmt.Println(data)

	// 2. Дополняем данные до кратности 8 байтам (64 битам)
	paddedData := PKCS7Pad(data, 8)
	fmt.Println(paddedData)

	// 3. Формируем срез 64-битных чисел
	blockCount := len(paddedData) / 8
	blocks := make([]uint64, blockCount)

	for i := 0; i < blockCount; i++ {
		// 8 байт
		chunk := paddedData[i*8 : (i+1)*8]
		// Big-Endian гарантирует, что 1-й байт станет старшими 8 битами uint64
		blocks[i] = binary.BigEndian.Uint64(chunk)
	}

	return blocks
}

func main() {

	str := "Test"

	fmt.Println(str)
	blocks := StringToBlocks(str)
	fmt.Println(blocks)

}
