#include "SutParser.h"

#include <cstdio>
#include <sstream>

#include "../scanner/Scanner.h"
#include "SemanticActions.h"
#include "SymbolTable.h"

//----------------------------------------------------------------------------------------------------------
bool SutParser::LoadTable(const std::string &path, std::string &err)
{
    return table_.Load(path, err);
}

//----------------------------------------------------------------------------------------------------------
bool SutParser::Parse(const std::string &text, SutResult &result)
{
    result = SutResult();

    Scanner scanner;
    std::vector<Token> tokens = scanner.RunScanner(text.c_str());
    if (tokens.empty())
    {
        result.errors.push_back("Сканер не вернул ни одного токена");
        return false;
    }

    SymbolTable sym(scanner.GetIdTable());

    // Имя программы — ключевое слово; отмечаем его категорию (действие A1 в грамматике отсутствует)
    int progName = sym.FindMain("IvanovIL");
    if (progName >= 0)
        sym.SetCat(progName, catProgName);

    std::vector<Attr> attrSt;
    std::vector<std::string> errors;
    Tac tac;
    SemanticActions actions(sym, tac, attrSt, errors);

    std::vector<int> states;
    states.push_back(1); // начальное состояние

    size_t pos = 0;
    Token token = tokens[0];
    Token pred = token;
    int symCol = table_.ColumnForToken(token.GetCode());

    if (token.GetCode() == lcErr)
    {
        errors.push_back("Лексическая ошибка: недопустимая лексема '" + token.GetLexeme() + "'");
    }

    bool stopped = false;
    bool fatal = false;
    int steps = 0;
    const int maxSteps = 200000;

    while (!stopped && !fatal && steps++ < maxSteps)
    {
        if (states.empty())
        {
            errors.push_back("Внутренняя ошибка: пуст стек состояний");
            fatal = true;
            break;
        }
        int state = states.back();

        if (symCol < 0)
        {
            errors.push_back("Для токена '" + token.GetLexeme() + "' нет столбца в таблице разбора");
            fatal = true;
            break;
        }

        const LRElem &e = table_.At(state, symCol);

        switch (e.type)
        {
        case LRElType::Err:
        {
            std::ostringstream os;
            os << "Синтаксическая ошибка: состояние " << state
               << ", недопустимый токен '" << token.GetLexeme() << "'";
            // Список допустимых терминалов в этом состоянии
            std::string allowed;
            for (int c = 1; c <= table_.ColCount(); ++c)
            {
                if (!table_.IsTerminal(c)) continue;
                const LRElem &x = table_.At(state, c);
                if (x.type != LRElType::Err)
                {
                    if (!allowed.empty()) allowed += ", ";
                    allowed += table_.ColName(c);
                }
            }
            if (!allowed.empty())
                os << "; допустимы: " << allowed;
            errors.push_back(os.str());
            fatal = true;
            break;
        }
        case LRElType::Shift:
        {
            states.push_back(e.par);
            if (table_.IsTerminal(symCol))
            {
                pred = token;
                if (pos + 1 < tokens.size())
                    token = tokens[++pos];
                if (token.GetCode() == lcErr)
                {
                    errors.push_back("Лексическая ошибка: недопустимая лексема '" + token.GetLexeme() + "'");
                    fatal = true;
                    break;
                }
                symCol = table_.ColumnForToken(token.GetCode());
            }
            else
            {
                // Перенос нетерминала: входной символ остаётся текущим токеном
                symCol = table_.ColumnForToken(token.GetCode());
            }
            break;
        }
        case LRElType::Reduct:
        {
            if ((int)states.size() <= e.par)
            {
                errors.push_back("Внутренняя ошибка: недостаточно состояний для свертки");
                fatal = true;
                break;
            }
            for (int i = 0; i < e.par; ++i)
                states.pop_back();
            symCol = e.left;
            if (e.act != 0)
                actions.Execute(e.act, pred);
            break;
        }
        case LRElType::Stop:
            stopped = true;
            break;
        }
    }

    if (!stopped && errors.empty())
        errors.push_back("Разбор не завершился (превышено число шагов)");

    result.ok = stopped && errors.empty();
    result.errors = errors;
    result.quads = tac.Quads();
    result.symbols = scanner.GetIdTable();
    for (int i = 0; i < sym.TableCount(); ++i)
        result.fieldTables.push_back(sym.Table(i));
    return result.ok;
}
