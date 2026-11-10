#include "LRTable.h"
#include "../third_party/pugixml/pugixml.hpp"

#include <cstdlib>
#include <cstring>

//----------------------------------------------------------------------------------------------------------
namespace
{
// Сопоставление имени столбца (лексемы) коду токена
int LexCodeByName(const std::string &name)
{
    if (name == "pckg") return lcPckg;
    if (name == "name") return lcIvanovIL;
    if (name == ";") return lcSemi;
    if (name == "eof") return lcEof;
    if (name == "typedef") return lcTypedef;
    if (name == "id") return lcId;
    if (name == "typeass") return lcTypeAss;
    if (name == "var") return lcVar;
    if (name == "struct") return lcStruct;
    if (name == "{") return lcStart;
    if (name == "}") return lcStop;
    if (name == "ass") return lcAss;
    if (name == ".") return lcDot;
    if (name == "do") return lcDo;
    if (name == "if") return lcIf;
    if (name == "(") return lcOpPar;
    if (name == ")") return lcClPar;
    if (name == "exit") return lcExit;
    if (name == "enddo") return lcEnddo;
    if (name == "comp") return lcComp;
    if (name == "add") return lcAdd;
    if (name == "or") return lcOr;
    if (name == "mult") return lcMult;
    if (name == "and") return lcAnd;
    if (name == "num") return lcNum;
    if (name == "true") return lcTrue;
    if (name == "false") return lcFalse;
    if (name == "not") return lcNot;
    return -1;
}

const LRElem kErrElem{};
} // namespace

//----------------------------------------------------------------------------------------------------------
bool LRTable::Load(const std::string &path, std::string &err)
{
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_file(path.c_str());
    if (!res)
    {
        err = std::string("не удалось разобрать XML: ") + res.description();
        return false;
    }

    pugi::xml_node root = doc.child("LR_Table");
    if (!root)
    {
        err = "в XML нет корневого элемента LR_Table";
        return false;
    }

    names_.clear();
    terminal_.clear();
    rows_.clear();
    lexCodeLR_.assign(LC_CNT, -1);

    pugi::xml_node cols = root.child("Columns");
    if (!cols)
    {
        err = "в XML нет раздела Columns";
        return false;
    }

    int maxCol = 0;
    // Нетерминалы
    for (pugi::xml_node c : cols.child("Neterms").children("Column"))
    {
        int n = c.attribute("ColNum").as_int();
        std::string lex = c.attribute("Lexeme").as_string();
        if (n > maxCol) maxCol = n;
        if ((int)names_.size() <= n)
        {
            names_.resize(n + 1);
            terminal_.resize(n + 1, false);
        }
        names_[n] = lex;
        terminal_[n] = false;
    }
    // Терминалы
    for (pugi::xml_node c : cols.child("Terms").children("Column"))
    {
        int n = c.attribute("ColNum").as_int();
        std::string lex = c.attribute("Lexeme").as_string();
        if (n > maxCol) maxCol = n;
        if ((int)names_.size() <= n)
        {
            names_.resize(n + 1);
            terminal_.resize(n + 1, false);
        }
        names_[n] = lex;
        terminal_[n] = true;
    }

    if (maxCol == 0)
    {
        err = "в XML нет столбцов";
        return false;
    }
    names_.resize(maxCol + 1);
    terminal_.resize(maxCol + 1, false);

    // Заполнение LexCodeLR: код токена -> столбец
    for (int c = 1; c <= maxCol; ++c)
    {
        if (!terminal_[c]) continue;
        int code = LexCodeByName(names_[c]);
        if (code >= 0 && code < LC_CNT)
            lexCodeLR_[code] = c;
    }

    // Строки таблицы
    int maxState = 0;
    for (pugi::xml_node row : root.children("Row"))
    {
        int st = row.attribute("NSost").as_int();
        if (st > maxState) maxState = st;
    }
    rows_.assign(maxState + 1, std::vector<LRElem>(maxCol + 1));

    for (pugi::xml_node row : root.children("Row"))
    {
        int st = row.attribute("NSost").as_int();
        for (pugi::xml_node e : row.children("ElemTR"))
        {
            int col = e.attribute("ColNum").as_int();
            if (col < 1 || col > maxCol) continue;
            LRElem el;
            el.type = (LRElType)e.attribute("ElType").as_int();
            el.par = e.attribute("ElPar").as_int();
            // В XML поле Left — имя нетерминала, а не номер столбца
            std::string leftName = e.attribute("Left").as_string();
            el.left = leftName.empty() ? 0 : FindColumn(leftName);
            el.numProd = e.attribute("NumProd").as_int();
            std::string act = e.attribute("Act").as_string();
            if (act.size() > 1 && (act[0] == 'A' || act[0] == 'a'))
                el.act = std::atoi(act.c_str() + 1);
            rows_[st][col] = el;
        }
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------
const LRElem &LRTable::At(int state, int col) const
{
    if (state < 0 || state >= (int)rows_.size())
        return kErrElem;
    if (col < 0 || col >= (int)rows_[state].size())
        return kErrElem;
    return rows_[state][col];
}

//----------------------------------------------------------------------------------------------------------
bool LRTable::IsTerminal(int col) const
{
    if (col < 0 || col >= (int)terminal_.size())
        return false;
    return terminal_[col];
}

//----------------------------------------------------------------------------------------------------------
int LRTable::ColumnForToken(LexCode code) const
{
    int idx = (int)code;
    if (idx < 0 || idx >= (int)lexCodeLR_.size())
        return -1;
    return lexCodeLR_[idx];
}

//----------------------------------------------------------------------------------------------------------
const std::string &LRTable::ColName(int col) const
{
    static const std::string empty;
    if (col < 0 || col >= (int)names_.size())
        return empty;
    return names_[col];
}

//----------------------------------------------------------------------------------------------------------
int LRTable::FindColumn(const std::string &name) const
{
    for (int i = 0; i < (int)names_.size(); ++i)
        if (names_[i] == name)
            return i;
    return -1;
}
