#ifndef SUT_SYMBOLTABLE_H
#define SUT_SYMBOLTABLE_H

//----------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "SutTypes.h"

//----------------------------------------------------------------------------------------------------------
// Таблица символов: основная таблица сканера + отдельные таблицы полей записей
class SymbolTable
{
public:
    explicit SymbolTable(std::vector<TableId_Elem> &main);

    int Count() const { return (int)main_.size(); }
    TableId_Elem &At(int index);
    const TableId_Elem &At(int index) const;

    int FindMain(const std::string &lex) const;
    int AddMain(const std::string &lex, CatId cat, const TypeInfo &type, int addr = -1);

    TypeInfo TypeOf(int pnt) const;
    void SetType(int pnt, CatId cat, const TypeInfo &type);
    void SetCat(int pnt, CatId cat);

    // Память данных
    int Alloc(int width);
    int EnsureAddr(int pnt); // выделить адрес константе/литералу при первом использовании
    int DataSize() const { return nextAddr_; }

    // Временные переменные и константы
    int NewTemp(const TypeInfo &type);
    int ConstZero(const TypeInfo &type);
    int ConstBool(bool value);

    // Таблицы полей записей
    int NewTable();
    int TableCount() const { return (int)fieldTables_.size(); }
    std::vector<TableId_Elem> &Table(int index);
    const std::vector<TableId_Elem> &Table(int index) const;
    int FindField(int table, const std::string &lex) const;
    int AddField(int table, const std::string &lex, const TypeInfo &type, int offset);

    // Адрес поля: отдельная запись основной таблицы
    int NewAddr(int basePnt, int offset, const TypeInfo &type, const std::string &lex = "");

private:
    std::vector<TableId_Elem> &main_;
    std::vector<std::vector<TableId_Elem>> fieldTables_;
    int nextAddr_ = 0;
    int tempNo_ = 0;
};

//----------------------------------------------------------------------------------------------------------
#endif // SUT_SYMBOLTABLE_H
