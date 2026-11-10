#include "SymbolTable.h"

#include <cstdio>

//----------------------------------------------------------------------------------------------------------
namespace
{
TableId_Elem g_dummy;
}

//----------------------------------------------------------------------------------------------------------
SymbolTable::SymbolTable(std::vector<TableId_Elem> &main) : main_(main)
{
}

//----------------------------------------------------------------------------------------------------------
TableId_Elem &SymbolTable::At(int index)
{
    if (index < 0 || index >= (int)main_.size())
        return g_dummy;
    return main_[index];
}

//----------------------------------------------------------------------------------------------------------
const TableId_Elem &SymbolTable::At(int index) const
{
    if (index < 0 || index >= (int)main_.size())
        return g_dummy;
    return main_[index];
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::FindMain(const std::string &lex) const
{
    for (int i = 0; i < (int)main_.size(); ++i)
        if (main_[i].GetLex() == lex)
            return i;
    return -1;
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::AddMain(const std::string &lex, CatId cat, const TypeInfo &type, int addr)
{
    main_.push_back(TableId_Elem(lex, cat, type.code, type.width, addr, type.fields));
    return (int)main_.size() - 1;
}

//----------------------------------------------------------------------------------------------------------
TypeInfo SymbolTable::TypeOf(int pnt) const
{
    TypeInfo t;
    const TableId_Elem &e = At(pnt);
    t.code = e.GetTypeCode();
    t.width = e.GetWidth();
    t.fields = e.GetFields();
    return t;
}

//----------------------------------------------------------------------------------------------------------
void SymbolTable::SetType(int pnt, CatId cat, const TypeInfo &type)
{
    if (pnt < 0 || pnt >= (int)main_.size())
        return;
    TableId_Elem &e = main_[pnt];
    e.SetCat(cat);
    e.SetTypeCode(type.code);
    e.SetWidth(type.width);
    e.SetFields(type.fields);
    if (cat == catVarName && e.GetAddr() < 0)
        e.SetAddr(Alloc(type.width > 0 ? type.width : 4));
}

//----------------------------------------------------------------------------------------------------------
void SymbolTable::SetCat(int pnt, CatId cat)
{
    if (pnt < 0 || pnt >= (int)main_.size())
        return;
    main_[pnt].SetCat(cat);
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::Alloc(int width)
{
    if (width <= 0)
        width = 4;
    int align = (width >= 8) ? 8 : 4;
    int addr = (nextAddr_ + align - 1) / align * align;
    nextAddr_ = addr + width;
    return addr;
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::EnsureAddr(int pnt)
{
    if (pnt < 0 || pnt >= (int)main_.size())
        return 0;
    TableId_Elem &e = main_[pnt];
    if (e.GetAddr() < 0)
    {
        int w = e.GetWidth() > 0 ? e.GetWidth() : 4;
        e.SetAddr(Alloc(w));
    }
    return e.GetAddr();
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::NewTemp(const TypeInfo &type)
{
    char name[32];
    std::snprintf(name, sizeof(name), "t%d", ++tempNo_);
    TypeInfo t = type;
    int addr = Alloc(t.width > 0 ? t.width : 4);
    int idx = AddMain(name, catVarName, t, addr);
    return idx;
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::ConstZero(const TypeInfo &type)
{
    std::string lex = (type.code == typeFloat) ? "0.0" : "0";
    int idx = FindMain(lex);
    if (idx >= 0)
    {
        if (main_[idx].GetAddr() < 0)
            main_[idx].SetAddr(Alloc(main_[idx].GetWidth() > 0 ? main_[idx].GetWidth() : 4));
        return idx;
    }
    return AddMain(lex, catConst, type, Alloc(type.width > 0 ? type.width : 4));
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::ConstBool(bool value)
{
    std::string lex = value ? "true" : "false";
    int idx = FindMain(lex);
    if (idx >= 0)
        return idx;
    TypeInfo t = LogicalType();
    return AddMain(lex, catConst, t, Alloc(t.width));
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::NewTable()
{
    fieldTables_.push_back(std::vector<TableId_Elem>());
    return (int)fieldTables_.size() - 1;
}

//----------------------------------------------------------------------------------------------------------
std::vector<TableId_Elem> &SymbolTable::Table(int index)
{
    static std::vector<TableId_Elem> empty;
    if (index < 0 || index >= (int)fieldTables_.size())
        return empty;
    return fieldTables_[index];
}

//----------------------------------------------------------------------------------------------------------
const std::vector<TableId_Elem> &SymbolTable::Table(int index) const
{
    static const std::vector<TableId_Elem> empty;
    if (index < 0 || index >= (int)fieldTables_.size())
        return empty;
    return fieldTables_[index];
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::FindField(int table, const std::string &lex) const
{
    if (table < 0 || table >= (int)fieldTables_.size())
        return -1;
    const std::vector<TableId_Elem> &t = fieldTables_[table];
    for (int i = 0; i < (int)t.size(); ++i)
        if (t[i].GetLex() == lex)
            return i;
    return -1;
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::AddField(int table, const std::string &lex, const TypeInfo &type, int offset)
{
    if (table < 0 || table >= (int)fieldTables_.size())
        return -1;
    fieldTables_[table].push_back(TableId_Elem(lex, catVarName, type.code, type.width, offset, type.fields));
    return (int)fieldTables_[table].size() - 1;
}

//----------------------------------------------------------------------------------------------------------
int SymbolTable::NewAddr(int basePnt, int offset, const TypeInfo &type, const std::string &lex)
{
    int baseAddr = At(basePnt).GetAddr();
    if (baseAddr < 0)
        baseAddr = 0;
    return AddMain(lex, catVarName, type, baseAddr + offset);
}
