#ifndef SUT_TAC_H
#define SUT_TAC_H

//----------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "../scanner/Token.h"

//----------------------------------------------------------------------------------------------------------
// Трехадресная команда (четверка)
struct Quad
{
    int op = opNone;
    int a1 = -1;
    int a2 = -1;
    int res = -1;
};

//----------------------------------------------------------------------------------------------------------
// Память трехадресных команд
class Tac
{
public:
    int Gen(int op, int a1, int a2, int res);
    int GenIf(int cond, int target);
    int GenGoto(int target);

    int Next() const { return (int)quads_.size(); }
    const std::vector<Quad> &Quads() const { return quads_; }

    static std::string OpName(int op);

private:
    std::vector<Quad> quads_;
};

//----------------------------------------------------------------------------------------------------------
#endif // SUT_TAC_H
