#include "Token.h"

//----------------------------------------------------------------------------------------------------------
std::string Token::GetTokenName_ByCode(LexCode code) const
{
    switch (code)
    {
    case lcPckg:
        return "pckg";
    case lcStart:
        return "{";
    case lcStop:
        return "}";
    case lcVar:
        return "var";
    case lcDo:
        return "do";
    case lcIf:
        return "if";
    case lcExit:
        return "exit";
    case lcEnddo:
        return "enddo";
    case lcTypedef:
        return "typedef";
    case lcStruct:
        return "struct";
    case lcInteger:
        return "integer";
    case lcFloat:
        return "float";
    case lcLogical:
        return "logical";
    case lcSemi:
        return ";";
    case lcDot:
        return ".";
    case lcOpPar:
        return "(";
    case lcClPar:
        return ")";
    case lcAss:
        return "ass";
    case lcTypeAss:
        return "typeass";
    case lcComp:
        return "comp";
    case lcAnd:
        return "and";
    case lcOr:
        return "or";
    case lcNot:
        return "not";
    case lcTrue:
        return "true";
    case lcFalse:
        return "false";
    case lcAdd:
        return "+";
    case lcMult:
        return "*";
    case lcId:
        return "id";
    case lcIvanovIL:
        return "IvanovIL";
    case lcNum:
        return "num";
    case lcEof:
        return "eof";
    case lcErr:
        return "err";
    default:
        return "UNKNOWN";
    } // switch
}

//----------------------------------------------------------------------------------------------------------