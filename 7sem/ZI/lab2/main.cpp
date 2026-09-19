#include <stdio.h>
#include <time.h>
#include <string>
#include <map>
#include <cstdlib>
#include <clocale>

#include "utils.h"

#define ALPHABET_SIZE 59
#define M 15
#define MAX_LEN 14
#define TESTS_COUNT 10

static const char alphabet[ALPHABET_SIZE] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 
    'u', 'v', 'w', 'x', 'y', 'z',
    'à', 'á', 'â', 'ã', 'ä', 'å', '¸', 'æ', 'ç', 'è', 
    'é', 'ê', 'ë', 'ì', 'í', 'î', 'ï', 'ð', 'ñ', 'ò', 
    'ó', 'ô', 'õ', 'ö', '÷', 'ø', 'ù', 'ú', 'û', 'ü', 'ý', 'þ', 'ÿ'
};

//--------------------------------------------------------------------------------------------
int CodeFunc(int i, int len)
{
    return (i + 1) % len;
}
// 
//--------------------------------------------------------------------------------------------
int DecodeFunc(int j, int len)
{
    return (j - 1 + len) % len;
}

typedef int(*CodeFuncPtr)(int, int);

//--------------------------------------------------------------------------------------------
std::string EncodeString(const std::string& s)
{
    std::string outputString = "";

    for (int i = 0; i < s.size(); i += M)
    {
        std::string groupStr = s.substr(i, M);
        std::string tempStr = "";
        tempStr.resize(M);
        
        for ( int j = 0; j < M; j++)
        {
            int newIndex = CodeFunc(j, M);
            tempStr[newIndex] = groupStr[j];
        }

        outputString += tempStr;
    }

    return outputString;
}

//--------------------------------------------------------------------------------------------
std::string DecodeString(const std::string& s)
{
    std::string outputString = "";

    for (int i = 0; i < s.size(); i += M)
    {
        std::string groupStr = s.substr(i, M);
        std::string tempStr = "";
        tempStr.resize(M);
        
        for ( int j = 0; j < M; j++)
        {
            int newIndex = DecodeFunc(j, M);
            tempStr[newIndex] = groupStr[j];
        }

        outputString += tempStr;
    }

    return outputString;
}

//--------------------------------------------------------------------------------------------
int main()
{
    setlocale(LC_ALL, "");
    srand(time(0));

    int strLen = 0;
    std::string inputStr = "";
    
    printf("%-7s %-30s %-30s %-30s\n", "Status", " InputStr", "   EncodeStr", "     DecodeStr");
    
    for (int i = 0; i < TESTS_COUNT; i++)
    {
        strLen = rand() % MAX_LEN + 1;

        inputStr = GenerateRandomString(alphabet, ALPHABET_SIZE, strLen);

        PadStrToMult(inputStr, alphabet[0], M);

        bool isCorrect = Test(inputStr, EncodeString, DecodeString);
    }
    
    return 0;
}
