#ifndef LC_TOKEN_H
#define LC_TOKEN_H

//----------------------------------------------------------------------------------------------------------
#include <string>

//----------------------------------------------------------------------------------------------------------
enum LcToken
{
    lcPckg = 0,  // package8
    lcStart = 1, // {
    lcStop,      // }
    lcVar,       // var8
    lcDo,        // do8
    lcIf,        // if8
    lcExit,      // exit8
    lcEnddo,     // enddo8
    lcTypedef,   // typedef8
    lcStruct,    // struct8
    lcInteger,   // integer
    lcFloat,     // float
    lcLogical,   // logical
    lcSemi,      // ;
    lcDot,       // .
    lcOpPar,     // (
    lcClPar,     // )
    lcAss,       // = (присваивание)
    lcTypeAss,   // <- (определение типа)
    lcComp,      // <, <=, >, >=, ==, != (операции сравнения)
    lcAnd,       // & (логичИ)
    lcOr,        // | (логичИЛИ)
    lcNot,       // ! (логичНЕ)
    lcTrue,      // true
    lcFalse,     // false
    lcAdd,       // +, - (аддитивные операции)
    lcMult,      // *, / (мультипликативные операции)
    lcId,        // идентификатор (буквы и цифры)
    lcNum,       // число (целое или вещественное)
    lcEof,       // маркер конца ввода
    lcErr,       // лексическая ошибка

    LC_CNT
};

//----------------------------------------------------------------------------------------------------------
enum TokenAttribute
{
    opNone = 0, // Нет атрибута (соответствует 0 в таблице)

    // Операции сравнения (lcComp)
    opL,  // <  (меньше)
    opLe, // <= (меньше или равно)
    opG,  // >  (больше)
    opGe, // >= (больше или равно)
    opE,  // == (равно)
    opNe, // != (не равно)

    // Аддитивные операции (lcAdd)
    opAdd, // +  (сложение)
    opSub, // -  (вычитание)

    // Мультипликативные операции (lcMult)
    opMult, // * (умножение)
    opDiv,  // /  (деление)

    OP_CNT
};

//----------------------------------------------------------------------------------------------------------
class Token
{
    LcToken code_;
    int32_t attr_;
    std::string lexeme_;

public:
    Token()
    {
    }
    Token(LcToken code, int32_t attr = opNone, std::string lexeme = "")
    {
        code_ = code;
        attr_ = attr;
        lexeme_ = lexeme;
    }
    ~Token()
    {
    }

    LcToken GetCode() const { return code_; }
    int32_t GetAttr() const { return attr_; }
    std::string GetLexeme() const { return lexeme_; }
    std::string GetTokenName_ByCode(LcToken code) const;

    // Сеттеры
    void SetCode(LcToken code) { code_ = code; }

    void SetLexeme(const std::string &lexeme) { lexeme_ = lexeme; }
};

//----------------------------------------------------------------------------------------------------------

#endif // LC_TOKEN_H