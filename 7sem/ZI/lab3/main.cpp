#include "utils.h"

#include <time.h>

#define ALPHABET_SIZE 256
#define TESTS_COUNT 10
#define MAX_LEN 50

#define B ALPHABET_SIZE
#define A 5
#define C 51
#define T0 13

//--------------------------------------------------------------------------------------------
// T(i+1)=(A*T(i)+C) mod B,
int T(u_int8_t i)
{
    if (i == 0)
        return T0;
    else
        return (A * T(i - 1) + C) % B;
}

//--------------------------------------------------------------------------------------------
u_int8_t CharToCode(char c)
{
    return (u_int8_t)c;
}

//--------------------------------------------------------------------------------------------
unsigned char CodeToChar(u_int8_t code)
{
    return (unsigned char)code;
}

//--------------------------------------------------------------------------------------------
std::string EncodeString(const std::string& inputStr)
{
    std::string outputStr(inputStr.size(), '\0');

    for (int i = 0; i < inputStr.size(); i++)
    {
        unsigned char c = inputStr[i];
        u_int8_t code = CharToCode(c);

        u_int8_t gamma = T(i);

        u_int8_t encodedCode = (code + gamma) % ALPHABET_SIZE;
        unsigned char encodedChar = CodeToChar(encodedCode);

        outputStr[i] = encodedChar;

        printf("Char: %c | Code: %3d | EncodedCode: %3d | EncodedChar: %c\n", c, code, encodedCode, encodedChar);
    }

    return outputStr;
}

//--------------------------------------------------------------------------------------------
std::string DecodeString(const std::string& encodedStr)
{
    std::string outputStr(encodedStr.size(), '\0');

    for (int i = 0; i < encodedStr.size(); i++)
    {
        unsigned char c = encodedStr[i];
        u_int8_t code = CharToCode(c);

        u_int8_t gamma = T(i);

        int temp = code - gamma;
        u_int8_t decodedCode = (temp >= 0) ? temp : (temp + ALPHABET_SIZE);
        unsigned char decodedChar = CodeToChar(decodedCode);

        outputStr[i] = decodedChar;

        printf("EncodedChar: %c | EncodedCode: %3d | DecodedCode: %3d | DecodedChar: %c\n", c, code, decodedCode, decodedChar);
    }

    return outputStr;
}

//--------------------------------------------------------------------------------------------
int main()
{
    setlocale(LC_ALL, "");
    srand(time(0));

    int strLen = 0;
    std::string inputStr = "";

    strLen = 50; //clrand() % MAX_LEN + 1;

    inputStr = GenerateRandomCharString(strLen);

    printf("\n");
    printf(inputStr.c_str());
    printf("\n");
    std::string encodedStr = EncodeString(inputStr);
    printf("\n");
    std::string decodedStr = DecodeString(encodedStr);
    printf(decodedStr.c_str());

    return 0;
}

//--------------------------------------------------------------------------------------------