#include "SemanticActions.h"

#include <cstdio>

//----------------------------------------------------------------------------------------------------------
SemanticActions::SemanticActions(SymbolTable &sym, Tac &tac, std::vector<Attr> &attrSt,
                                 std::vector<std::string> &errors)
    : sym_(sym), tac_(tac), attrSt_(attrSt), errors_(errors)
{
}

//----------------------------------------------------------------------------------------------------------
Attr SemanticActions::pop()
{
    if (attrSt_.empty())
        return Attr::makeInt(0);
    Attr a = attrSt_.back();
    attrSt_.pop_back();
    return a;
}

//----------------------------------------------------------------------------------------------------------
int SemanticActions::popInt()
{
    Attr a = pop();
    return (a.kind == Attr::Int) ? a.value : 0;
}

//----------------------------------------------------------------------------------------------------------
TypeInfo SemanticActions::popType()
{
    Attr a = pop();
    return (a.kind == Attr::Type) ? a.type : TypeInfo();
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::error(int code, const std::string &msg)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "A%d", currentAct_);
    errors_.push_back(std::string("Ошибка ") + std::to_string(code) + ": " + msg + " (" + buf + ")");
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::Execute(int act, const Token &predToken)
{
    currentAct_ = act;
    switch (act)
    {
    case 2: a2(predToken); break;
    case 3: a3(); break;
    case 4: a4(predToken); break;
    case 5: a5(predToken); break;
    case 6: a6(); break;
    case 9: a9(predToken); break;
    case 11: a11(); break;
    case 12: a12(predToken); break;
    case 13: a13(predToken); break;
    case 14: a14(); break;
    case 15: a15(predToken); break;
    case 16: a16(); break;
    case 17: a17(predToken); break;
    case 18: a18(); break;
    case 19: a19(predToken); break;
    case 20: a20(); break;
    case 21: a21(); break;
    case 22: a22(predToken); break;
    case 23: a23(); break;
    case 24: a24(); break;
    case 25: a25(predToken); break;
    case 26: a26(); break;
    case 27: a27(); break;
    case 28: a28(); break;
    case 29: a29(); break;
    case 30: a30(); break;
    case 31: a31(); break;
    case 32: a32(); break;
    default: break;
    }
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a2(const Token &t)
{
    int pnt = t.GetAttr();
    if (sym_.At(pnt).GetCat() != catNoCat)
        error(1, "повторное объявление идентификатора " + t.GetLexeme());
    pushInt(pnt);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a3()
{
    TypeInfo type = popType();
    int pnt = popInt();
    sym_.SetType(pnt, catTypeName, type);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a4(const Token &t)
{
    TypeInfo type = popType();
    int pnt = t.GetAttr();
    if (sym_.At(pnt).GetCat() != catNoCat)
        error(1, "повторное объявление идентификатора " + t.GetLexeme());
    sym_.SetType(pnt, catVarName, type);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a5(const Token &t)
{
    int pnt = t.GetAttr();
    if (sym_.At(pnt).GetCat() != catTypeName)
        error(2, "идентификатор " + t.GetLexeme() + " не является именем типа");
    pushType(sym_.TypeOf(pnt));
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a6()
{
    if (ST_.empty())
    {
        error(8, "нет таблицы полей для записи");
        pushType(TypeInfo());
        return;
    }
    int table = ST_.back();
    ST_.pop_back();
    int savedOffset = SO_.empty() ? 0 : SO_.back();
    if (!SO_.empty())
        SO_.pop_back();
    TypeInfo t;
    t.code = typeRecord;
    t.width = offset_;
    t.fields = table;
    offset_ = savedOffset;
    pushType(t);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a9(const Token &t)
{
    TypeInfo type = popType();
    if (ST_.empty())
    {
        error(8, "поле объявлено вне записи");
        return;
    }
    int table = ST_.back();
    if (sym_.FindField(table, t.GetLexeme()) >= 0)
        error(1, "повторное объявление поля " + t.GetLexeme());
    sym_.AddField(table, t.GetLexeme(), type, offset_);
    offset_ += type.width > 0 ? type.width : 4;
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a11()
{
    int exprAddr = popInt();
    TypeInfo exprType = popType();
    int varAddr = popInt();
    TypeInfo varType = popType();
    if (!TypeEqual(varType, exprType))
        error(4, "несовместимость типов при присваивании");
    tac_.Gen(opAss, exprAddr, -1, varAddr);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a12(const Token &t)
{
    int pnt = t.GetAttr();
    if (sym_.At(pnt).GetCat() == catNoCat)
        error(5, "идентификатор " + t.GetLexeme() + " не объявлен");
    else if (sym_.At(pnt).GetCat() != catVarName)
        error(3, "идентификатор " + t.GetLexeme() + " не является переменной");
    pushType(sym_.TypeOf(pnt));
    pushInt(pnt);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a13(const Token &t)
{
    pushInt(t.GetAttr()); // add.op
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a14()
{
    int termAddr = popInt();
    TypeInfo termType = popType();
    int op = popInt();
    if (termType.code == typeLogical)
        error(6, "операнд арифметической операции должен быть целым или вещественным");
    pushType(termType);
    if (op == opSub)
    {
        int tmp = sym_.NewTemp(termType);
        pushInt(tmp);
        tac_.Gen(opSub, sym_.ConstZero(termType), termAddr, tmp);
    }
    else
    {
        pushInt(termAddr);
    }
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a15(const Token &t)
{
    pushInt(t.GetAttr()); // add.op
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a16()
{
    int a2 = popInt();
    TypeInfo t2 = popType();
    int op = popInt();
    int a1 = popInt();
    TypeInfo t1 = popType();
    if (!TypeEqual(t1, t2))
        error(4, "несовместимость типов операндов");
    if (t1.code == typeLogical)
        error(6, "операнд арифметической операции должен быть целым или вещественным");
    pushType(t2);
    int tmp = sym_.NewTemp(t2);
    pushInt(tmp);
    tac_.Gen(op, a1, a2, tmp);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a17(const Token &t)
{
    pushInt(t.GetAttr()); // mult.op
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a18()
{
    int a2 = popInt();
    TypeInfo t2 = popType();
    int op = popInt();
    int a1 = popInt();
    TypeInfo t1 = popType();
    if (!TypeEqual(t1, t2))
        error(4, "несовместимость типов операндов");
    if (t1.code == typeLogical)
        error(6, "операнд арифметической операции должен быть целым или вещественным");
    pushType(t2);
    int tmp = sym_.NewTemp(t2);
    pushInt(tmp);
    tac_.Gen(op, a1, a2, tmp);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a19(const Token &t)
{
    int pnt = t.GetAttr();
    TypeInfo type = sym_.TypeOf(pnt);
    sym_.EnsureAddr(pnt);
    pushType(type);
    pushInt(pnt);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a20()
{
    pushType(LogicalType());
    pushInt(sym_.ConstBool(true));
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a21()
{
    pushType(LogicalType());
    pushInt(sym_.ConstBool(false));
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a22(const Token &t)
{
    int varAddr = popInt();
    TypeInfo varType = popType();
    if (varType.code != typeRecord)
    {
        error(8, "обращение к полю не у записи");
        pushType(TypeInfo());
        pushInt(varAddr);
        return;
    }
    int f = sym_.FindField(varType.fields, t.GetLexeme());
    if (f < 0)
    {
        error(9, "поле " + t.GetLexeme() + " не найдено");
        pushType(TypeInfo());
        pushInt(varAddr);
        return;
    }
    const TableId_Elem &fe = sym_.Table(varType.fields)[f];
    TypeInfo ft;
    ft.code = fe.GetTypeCode();
    ft.width = fe.GetWidth();
    ft.fields = fe.GetFields();
    std::string addrName = sym_.At(varAddr).GetLex() + "." + t.GetLexeme();
    int addr = sym_.NewAddr(varAddr, fe.GetAddr(), ft, addrName);
    pushType(ft);
    pushInt(addr);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a23()
{
    pushInt(tac_.Next());
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a24()
{
    int exprAddr = popInt();
    TypeInfo exprType = popType();
    int m = popInt();
    if (exprType.code != typeLogical)
        error(7, "условие цикла должно быть логического типа");
    int target = tac_.Next() + 2;
    tac_.GenIf(exprAddr, target);
    tac_.GenGoto(m);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a25(const Token &t)
{
    pushInt(t.GetAttr()); // comp.op
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a26()
{
    int a2 = popInt();
    TypeInfo t2 = popType();
    int op = popInt();
    int a1 = popInt();
    TypeInfo t1 = popType();
    if (!TypeEqual(t1, t2))
        error(4, "несовместимость типов при сравнении");
    if (t1.code == typeLogical && op != opE && op != opNe)
        error(6, "для логических операндов допустимы только == и !=");
    TypeInfo lt = LogicalType();
    pushType(lt);
    int tmp = sym_.NewTemp(lt);
    pushInt(tmp);
    tac_.Gen(op, a1, a2, tmp);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a27()
{
    int a2 = popInt();
    TypeInfo t2 = popType();
    int a1 = popInt();
    TypeInfo t1 = popType();
    if (!TypeEqual(t1, t2))
        error(4, "несовместимость типов операндов");
    if (t1.code != typeLogical)
        error(7, "операнд операции or должен быть логическим");
    TypeInfo lt = LogicalType();
    pushType(lt);
    int tmp = sym_.NewTemp(lt);
    pushInt(tmp);
    tac_.Gen(opOr, a1, a2, tmp);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a28()
{
    a27();
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a29()
{
    int a2 = popInt();
    TypeInfo t2 = popType();
    int a1 = popInt();
    TypeInfo t1 = popType();
    if (!TypeEqual(t1, t2))
        error(4, "несовместимость типов операндов");
    if (t1.code != typeLogical)
        error(7, "операнд операции and должен быть логическим");
    TypeInfo lt = LogicalType();
    pushType(lt);
    int tmp = sym_.NewTemp(lt);
    pushInt(tmp);
    tac_.Gen(opAnd, a1, a2, tmp);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a30()
{
    a29();
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a31()
{
    int a = popInt();
    TypeInfo t = popType();
    if (t.code != typeLogical)
        error(7, "операнд операции not должен быть логическим");
    TypeInfo lt = LogicalType();
    pushType(lt);
    int tmp = sym_.NewTemp(lt);
    pushInt(tmp);
    tac_.Gen(opNot, a, -1, tmp);
}

//----------------------------------------------------------------------------------------------------------
void SemanticActions::a32()
{
    int table = sym_.NewTable();
    ST_.push_back(table);
    SO_.push_back(offset_);
    offset_ = 0;
}
