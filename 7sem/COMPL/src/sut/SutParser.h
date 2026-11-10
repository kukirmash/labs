#ifndef SUT_PARSER_H
#define SUT_PARSER_H

//----------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "LRTable.h"
#include "SutTypes.h"
#include "Tac.h"

//----------------------------------------------------------------------------------------------------------
// Результат работы СУ-транслятора
struct SutResult
{
    bool ok = false;
    std::vector<std::string> errors;
    std::vector<Quad> quads;
    std::vector<TableId_Elem> symbols;
    std::vector<std::vector<TableId_Elem>> fieldTables;
};

//----------------------------------------------------------------------------------------------------------
// LR-анализатор с синтаксически управляемой трансляцией
class SutParser
{
public:
    bool LoadTable(const std::string &path, std::string &err);
    bool Parse(const std::string &text, SutResult &result);

    const LRTable &Table() const { return table_; }

private:
    LRTable table_;
};

//----------------------------------------------------------------------------------------------------------
#endif // SUT_PARSER_H
