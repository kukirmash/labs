#ifndef SUT_SEMANTICACTIONS_H
#define SUT_SEMANTICACTIONS_H

//----------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "SutTypes.h"
#include "SymbolTable.h"
#include "Tac.h"

//----------------------------------------------------------------------------------------------------------
// Реализация семантических действий A2..A32 (A1, A7, A8, A10 в грамматике отсутствуют)
class SemanticActions
{
public:
    SemanticActions(SymbolTable &sym, Tac &tac, std::vector<Attr> &attrSt,
                    std::vector<std::string> &errors);

    void Execute(int act, const Token &predToken);

private:
    SymbolTable &sym_;
    Tac &tac_;
    std::vector<Attr> &attrSt_;
    std::vector<std::string> &errors_;

    std::vector<int> ST_; // стек таблиц полей
    std::vector<int> SO_; // стек сохранённых offset
    int offset_ = 0;      // текущее смещение поля

    int currentAct_ = 0;

    void pushInt(int v) { attrSt_.push_back(Attr::makeInt(v)); }
    void pushType(const TypeInfo &t) { attrSt_.push_back(Attr::makeType(t)); }
    Attr pop();
    int popInt();
    TypeInfo popType();

    void error(int code, const std::string &msg);

    void a2(const Token &t);
    void a3();
    void a4(const Token &t);
    void a5(const Token &t);
    void a6();
    void a9(const Token &t);
    void a11();
    void a12(const Token &t);
    void a13(const Token &t);
    void a14();
    void a15(const Token &t);
    void a16();
    void a17(const Token &t);
    void a18();
    void a19(const Token &t);
    void a20();
    void a21();
    void a22(const Token &t);
    void a23();
    void a24();
    void a25(const Token &t);
    void a26();
    void a27();
    void a28();
    void a29();
    void a30();
    void a31();
    void a32();
};

//----------------------------------------------------------------------------------------------------------
#endif // SUT_SEMANTICACTIONS_H
