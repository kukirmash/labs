#include "utils.h"

#include <cstdlib>
#include <cstdio>

//--------------------------------------------------------------------------------------------
std::string GenerateRandomString(const char *alphabet, int alphabetSize, int strLen)
{
    std::string answer(strLen, '\0');

    for (int i = 0; i < strLen; i++)
    {
        answer[i] = alphabet[rand() % alphabetSize];
    }

    return answer;
}

//--------------------------------------------------------------------------------------------
std::string GenerateRandomCharString(int strLen)\
{
    std::string answer(strLen, '\0');

    for (int i = 0; i < strLen; i++)
    {
        answer[i] = (char)(rand() % 256);
    }

    return answer;
}

//--------------------------------------------------------------------------------------------
void PadStrToMult(std::string &str, const char sym, int multLen)
{
    if (multLen <= 0)
        return;

    size_t rem = str.size() % multLen;
    if (rem != 0)
        str.append(multLen - rem, sym);
}

//--------------------------------------------------------------------------------------------
bool Test(const std::string& inputStr, TransformStringFunc encodeFunc, TransformStringFunc decodeFunc)
{
    std::string encodedStr = encodeFunc(inputStr);
    std::string decodedStr = decodeFunc(encodedStr);

    bool isEqual = (inputStr == decodedStr);

    const char *color = isEqual ? "\033[32m" : "\033[31m";
    const char *reset = "\033[0m";
    const char *status = isEqual ? "OK" : "Error";

    printf("%s%-7s%s [%-30s] [%-30s] [%-30s]\n", color, status, reset, inputStr.c_str(), encodedStr.c_str(), decodedStr.c_str());

    return isEqual;
}

//--------------------------------------------------------------------------------------------