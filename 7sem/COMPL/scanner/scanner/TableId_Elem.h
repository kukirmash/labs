#ifndef ID_H
#define ID_H

//----------------------------------------------------------------------------------------------------------
#include <string>

//----------------------------------------------------------------------------------------------------------
// Категории идентификаторов и констант
enum CatId
{
    catNoCat = 0, // категория ещё не определена
    catProgName,  // имя программы
    catTypeName,  // имя типа
    catVarName,   // имя переменной
    catConst      // константа
};

//----------------------------------------------------------------------------------------------------------
// Коды типов данных
enum TypeCode
{
    typeVoid = 0,
    typeInt,
    typeFloat,
    typeLogical,
    typeRecord
};

//----------------------------------------------------------------------------------------------------------
// Размеры предопределённых типов (в байтах)
const int wInt = 4;
const int wFloat = 8;
const int wLogical = 4;

//----------------------------------------------------------------------------------------------------------
class TableId_Elem
{
    std::string lex_;   // лексема
    CatId cat_;         // категория
    TypeCode typeCode_; // код типа
    int width_;         // размер типа
    int addr_;          // адрес в памяти данных (-1, если не выделен)

public:
    TableId_Elem(const std::string &lex = "", CatId cat = catNoCat,
                 TypeCode typeCode = typeVoid, int width = 0, int addr = -1)
        : lex_(lex), cat_(cat), typeCode_(typeCode), width_(width), addr_(addr)
    {
    }
    ~TableId_Elem()
    {
    }

    // Геттеры
    const std::string &GetLex() const { return lex_; }
    CatId GetCat() const { return cat_; }
    TypeCode GetTypeCode() const { return typeCode_; }
    int GetWidth() const { return width_; }
    int GetAddr() const { return addr_; }

    // Сеттеры
    void SetLex(const std::string &lex) { lex_ = lex; }
    void SetCat(CatId cat) { cat_ = cat; }
    void SetTypeCode(TypeCode typeCode) { typeCode_ = typeCode; }
    void SetWidth(int width) { width_ = width; }
    void SetAddr(int addr) { addr_ = addr; }
};

//----------------------------------------------------------------------------------------------------------
#endif // ID_H
