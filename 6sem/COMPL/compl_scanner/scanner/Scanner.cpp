#include "Scanner.h"
#include <cctype> // Для std::isalpha и std::isdigit

//----------------------------------------------------------------------------------------------------------
std::vector<Token> Scanner::RunScanner(const char *text)
{
    std::vector<Token> tokens;
    u_int32_t i = 0;

    while (text[i] != '\0')
    {
        int8_t currState = 0;
        std::string currLexeme = "";

        while (currState >= 0)
        {
            const char sym = text[i];
            currLexeme += sym;

            u_int32_t rowIdx = this->GetRowIndex_BySymbol(sym);
            currState = transitionTable[rowIdx][currState];

            i++;
        }

        // Проверяем, нужно ли делать возврат (retract) на 1 символ назад
        bool isRetract = (currState == -11 || currState == -13 ||
                          currState == -14 || currState == -15 ||
                          currState == -16 || currState == -19 ||
                          currState == -21 || currState == -23);

        if (isRetract)
        {
            i = i - 1;
            currLexeme.pop_back();
        }

        // пробел и комментарий - токен не генерируется
        if (currState == -16 || currState == -18)
            continue;

        // 1. Получаем базовый токен по состоянию автомата
        Token token = this->GetToken_ByState(currState);
        token.SetLexName(currLexeme);

        // 2. Уточняем: если это идентификатор, проверим словарь ключевых слов
        if (currState == -13)
        {
            auto it = keywordMap.find(currLexeme);

            if (it != keywordMap.end())
            {
                // Слово нашлось в словаре! Заменяем lcId на точный код
                token.SetCode(it->second);
            }
            else
            {
                // Это переменная пользователя (id). Работаем с таблицей идентификаторов.
                int idIndex = -1;

                // Шаг А: Ищем лексему в таблице идентификаторов
                for (size_t j = 0; j < idTable.size(); ++j)
                {
                    if (idTable[j].GetName() == currLexeme)
                    {
                        idIndex = idTable[j].GetId();
                        break;
                    }
                }

                // Шаг Б: Если не нашли, добавляем новую запись
                if (idIndex == -1)
                {
                    idIndex = idTable.size(); // Индекс равен текущему размеру массива
                    idTable.push_back(Id(idIndex, currLexeme));
                }

                // Шаг В: Устанавливаем атрибут токена равным его номеру в таблице
                token.SetAttr(idIndex);
            }
        }

        tokens.push_back(token);

        // ошибка
        if (currState == -27 || currState == -28)
            return tokens;
    }

    return tokens;
}

//----------------------------------------------------------------------------------------------------------
u_int32_t Scanner::GetRowIndex_BySymbol(const char &ch)
{
    unsigned char uc = static_cast<unsigned char>(ch);
    if (std::isalpha(uc))
        return 14;
    if (std::isdigit(uc))
        return 15;

    switch (ch)
    {
    case ';':
        return 0;
    case '.':
        return 1;
    case '(':
        return 2;
    case ')':
        return 3;
    case '+':
        return 4;
    case '-':
        return 5;
    case '*':
        return 6;
    case '/':
        return 7;
    case '=':
        return 8;
    case '<':
        return 9;
    case '>':
        return 10;
    case '!':
        return 11;
    case '&':
        return 12;
    case '|':
        return 13;
    case '{':
        return 16;
    case '}':
        return 17;
    case ' ':
        return 18; // Пробел
    case '\t':
        return 19; // Табуляция
    case '\n':     // Перевод строки
    case '\r':
        return 20; // Возврат каретки
    default:
        return 21; // Любой другой символ
    }
}

//----------------------------------------------------------------------------------------------------------
Token Scanner::GetToken_ByState(int8_t state)
{
    // Отрицательные состояния в таблице переходов являются конечными.
    // Они указывают, какой именно токен мы только что распознали.

    switch (state)
    {
    case -1:
        return Token(lcSemi); // ;
    case -2:
        return Token(lcDot); // .
    case -3:
        return Token(lcOpPar); // (
    case -4:
        return Token(lcClPar); // )
    case -5:
        return Token(lcAnd);
    case -6:
        return Token(lcOr);
    case -7:
        return Token(lcAdd, opAdd); // +
    case -8:
        return Token(lcAdd, opSub); // -
    case -9:
        return Token(lcMult, opMult); // *
    case -10:
        return Token(lcMult, opDiv); // /
    case -11:
        return Token(lcAss); // = (присваивание)
    case -12:
        return Token(lcComp, opE); // == (равно)
    case -13:
        return Token(lcId); // Идентификатор (или ключевое слово)
    case -14:
        return Token(lcNum); // Целое число
    case -15:
        return Token(lcNum); // Вещественное число
    case -17:
        return Token(lcComp, opLe); // <=
    case -19:
        return Token(lcComp, opL); // <
    case -20:
        return Token(lcComp, opGe); // >=
    case -21:
        return Token(lcComp, opG); // >
    case -22:
        return Token(lcComp, opNe); // !=
    case -23:
        return Token(lcNot); // !
    case -24:
        return Token(lcStart); // {
    case -25:
        return Token(lcStop); // }
    case -26:
        return Token(lcTypeAss);
    case -27: // err(1) - лексема не может начинаться с этого символа
    case -28: // err(2) - неправильный формат числа (
        return Token(lcErr);

    default:
        return Token(lcErr); // Неизвестное состояние
    }
}

//----------------------------------------------------------------------------------------------------------