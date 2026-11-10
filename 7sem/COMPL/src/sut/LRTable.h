#ifndef SUT_LRTABLE_H
#define SUT_LRTABLE_H

//----------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "SutTypes.h"

//----------------------------------------------------------------------------------------------------------
// Таблица разбора, загружаемая из LR_Table.xml (pugixml)
class LRTable
{
public:
    // Загружает таблицу; при ошибке возвращает false и заполняет err
    bool Load(const std::string &path, std::string &err);

    int StateCount() const { return (int)rows_.size(); }
    int ColCount() const { return (int)names_.size(); }

    // Элемент таблицы; для недопустимых индексов возвращается элемент ошибки
    const LRElem &At(int state, int col) const;

    bool IsTerminal(int col) const;
    int ColumnForToken(LexCode code) const;
    const std::string &ColName(int col) const;
    int FindColumn(const std::string &name) const;

private:
    std::vector<std::string> names_;      // имена столбцов
    std::vector<bool> terminal_;          // true — терминал
    std::vector<std::vector<LRElem>> rows_; // [состояние][столбец]
    std::vector<int> lexCodeLR_;          // код токена -> номер столбца
};

//----------------------------------------------------------------------------------------------------------
#endif // SUT_LRTABLE_H
