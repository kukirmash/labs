#include "Tac.h"

//----------------------------------------------------------------------------------------------------------
int Tac::Gen(int op, int a1, int a2, int res)
{
    Quad q;
    q.op = op;
    q.a1 = a1;
    q.a2 = a2;
    q.res = res;
    quads_.push_back(q);
    return (int)quads_.size() - 1;
}

//----------------------------------------------------------------------------------------------------------
int Tac::GenIf(int cond, int target)
{
    Quad q;
    q.op = opIfGoto;
    q.a1 = cond;
    q.res = target;
    quads_.push_back(q);
    return (int)quads_.size() - 1;
}

//----------------------------------------------------------------------------------------------------------
int Tac::GenGoto(int target)
{
    Quad q;
    q.op = opGoto;
    q.res = target;
    quads_.push_back(q);
    return (int)quads_.size() - 1;
}

//----------------------------------------------------------------------------------------------------------
std::string Tac::OpName(int op)
{
    switch (op)
    {
    case opAdd: return "+";
    case opSub: return "-";
    case opMult: return "*";
    case opDiv: return "/";
    case opAss: return "=";
    case opOr: return "or";
    case opAnd: return "and";
    case opNot: return "not";
    case opL: return "<";
    case opLe: return "<=";
    case opG: return ">";
    case opGe: return ">=";
    case opE: return "==";
    case opNe: return "!=";
    case opIfGoto: return "if goto";
    case opGoto: return "goto";
    default: return "?";
    }
}
