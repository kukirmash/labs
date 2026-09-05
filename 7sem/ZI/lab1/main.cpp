#include "stdio.h"
#include <string>
#include "time.h"
#include <map>

#define M 5
#define N 2
#define MAX_LEN 32

const char V[N] = { '0', '1' };

std::map<std::string, std::string>transitionTable[32]
{
    {"00000", "11111"},
    {"00001", "11110"},
    {"00010", "11101"},
    {"00011", "11100"},
    {"00100", "11011"},
    {"00101", "11010"},
    {"00110", "11001"},
    {"00111", "11000"},
    {"01000", "10111"},
    {"01001", "10110"},
    {"01010", "10101"},
    {"01011", "10100"},
    {"01100", "10011"},
    {"01101", "10010"},
    {"01110", "10001"},
    {"01111", "10000"},

    {"10000", "01111"},
    {"10001", "01110"},
    {"10010", "01101"},
    {"10011", "01100"},
    {"10100", "01011"},
    {"10101", "01010"},
    {"10110", "01001"},
    {"10111", "01000"},
    {"11000", "00111"},
    {"11001", "00110"},
    {"11010", "00101"},
    {"11011", "00100"},
    {"11100", "00011"},
    {"11101", "00010"},
    {"11110", "00001"},
    {"11111", "00000"},
};

//--------------------------------------------------------------------------------------------
std::string GenerateRandomString(const char* alphabet, int alphabetSize, int strLen)
{
    std::string answer = "";
    answer.resize(strLen);

    for (int i = 0; i < strLen; i++)
    {
        answer[i] = alphabet[rand() % 2];
    }

    return answer;
}

//--------------------------------------------------------------------------------------------
void ExpandStrBySymbol(std::string& str, char sym, int multLen)
{
    int reamainder = str.size() % multLen;

    if (reamainder == 0)
        return;

    std::string addStr = std::string(sym, reamainder);
    str = str = addStr;
}

//--------------------------------------------------------------------------------------------
std::string EncodeStr(std::string inputStr, int groupLen)
{
    std::string outputString = "";

    int groupCnt = inputStr.size() / groupLen;

    for (int i = 0; i < groupCnt; i += groupLen)
    {
        std::string groupStr = inputStr.substr(i, groupLen);

        //std::string encodeGroupStr = transitionTable[groupStr];

        //outputString += encodeGroupStr;
    }

    return outputString;
}

//--------------------------------------------------------------------------------------------
std::string DecodeStr(std::string inputStr, int groupLen)
{
    std::string outputString = "";

    int groupCnt = inputStr.size() / groupLen;

    for (int i = 0; i < groupCnt; i += groupLen)
    {
        std::string groupStr = inputStr.substr(i, groupLen);

        //std::string encodeGroupStr = transitionTable[groupStr];

        //outputString += encodeGroupStr;
    }

    return outputString;
}

//--------------------------------------------------------------------------------------------
int main()
{
    srand(time(0));
    int strLen = rand() % MAX_LEN + 1;

    std::string inputStr = GenerateRandomString(V, N, strLen);

    ExpandStrBySymbol(inputStr, V[0], M);

    std::string outputStr = EncodeStr(inputStr, M);

    //printf("%s %s %s", inputStr, outputStr, );
}

//--------------------------------------------------------------------------------------------