#include "stdio.h"
#include "time.h"
#include <string>
#include <map>

#define ALPHABET_SIZE 59
#define M 15
#define MAX_LEN 14
#define TESTS_COUNT 10

static const wchar_t alphabet[ALPHABET_SIZE] = {
    L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h', L'i', L'j', 
    L'k', L'l', L'm', L'n', L'o', L'p', L'q', L'r', L's', L't', 
    L'u', L'v', L'w', L'x', L'y', L'z',
    L'а', L'б', L'в', L'г', L'д', L'е', L'ё', L'ж', L'з', L'и', 
    L'й', L'к', L'л', L'м', L'н', L'о', L'п', L'р', L'с', L'т', 
    L'у', L'ф', L'х', L'ц', L'ч', L'ш', L'щ', L'ъ', L'ы', L'ь', L'э', L'ю', L'я'
};

//--------------------------------------------------------------------------------------------
std::wstring GenerateRandomString(const wchar_t *alphabet, int alphabetSize, int strLen)
{
    std::wstring answer = L"";
    answer.resize(strLen);

    for (int i = 0; i < strLen; i++)
    {
        int randomIndex = rand() % alphabetSize;
        wchar_t randomChar = alphabet[randomIndex];
        answer[i] = randomChar;
    }

    return answer;
}

//--------------------------------------------------------------------------------------------
void ExpandStrBySymbol(std::wstring &str, const wchar_t sym, int multLen)
{
    int diff = 0;
    if (str.size() < multLen)
    {
        diff = multLen - str.size();
    }
    else
    {
        int remainder = str.size() % multLen;
        diff = multLen - remainder;
    }

    if (diff == 0)
        return;

    std::wstring addStr = std::wstring(diff, sym);
    str = str + addStr;
    str.shrink_to_fit();
}

//--------------------------------------------------------------------------------------------
int CodeFunc(int i, int strLen)
{
    return (i + 1) % strLen;
}

//--------------------------------------------------------------------------------------------
int DecodeFunc(int j, int strLen)
{
    return (j - 1 + strLen) % strLen;
}

typedef int(*CodeFuncPtr)(int, int);

//--------------------------------------------------------------------------------------------
std::wstring CodeStr(std::wstring inputStr, int groupLen, CodeFuncPtr codeFunc)
{
    std::wstring outputString = L"";

    for (int i = 0; i < inputStr.size(); i += groupLen)
    {
        std::wstring groupStr = inputStr.substr(i, groupLen);
        std::wstring tempStr = L"";
        tempStr.resize(groupLen);
        
        for ( int j = 0; j < groupLen; j++)
        {
            int newIndex = codeFunc(j, groupLen);
            tempStr[j] = groupStr[newIndex];
        }

        outputString += tempStr;
    }

    return outputString;
}

//--------------------------------------------------------------------------------------------
bool Test(std::wstring inputStr, int groupLen, CodeFuncPtr CodeFunc, CodeFuncPtr DecodeFunc)
{
    std::wstring outputStr = CodeStr(inputStr, groupLen, CodeFunc);

    std::wstring decodeStr = CodeStr(outputStr, groupLen, DecodeFunc);

    bool isEqual = (inputStr == decodeStr);

    const wchar_t *color = isEqual ? L"\033[32m" : L"\033[31m"; 
    const wchar_t *reset = L"\033[0m";                         
    const wchar_t *status = isEqual ? L"\033[32mOK\033[0m" : L"\033[31mError\033[0m";

    fwprintf(stdout, L"%ls%-7ls%ls [%-20ls] [%-20ls] [%-20ls]\n", color, status, reset, inputStr.c_str(), outputStr.c_str(), decodeStr.c_str());

    return isEqual;
}

//--------------------------------------------------------------------------------------------
int main()
{
    setlocale(LC_ALL, "");
    srand(time(0));

    int strLen = 0;
    std::wstring inputStr = L"";
    fwprintf(stdout, L"%-7ls %-20ls %-20ls %-20ls\n", L"Status", L" InputStr", L"   EncodeStr", L"     DecodeStr");
    for (int i = 0; i < TESTS_COUNT; i++)
    {
        strLen = rand() % MAX_LEN + 1;

        inputStr = GenerateRandomString(alphabet, ALPHABET_SIZE, strLen);

        ExpandStrBySymbol(inputStr, alphabet[0], M);

        bool isCorrect = Test(inputStr, M, CodeFunc, DecodeFunc);
    }
}

//--------------------------------------------------------------------------------------------