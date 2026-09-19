#ifndef UTILS_H
#define UTILS_H

#include <string>

//--------------------------------------------------------------------------------------------
typedef std::string(*TransformStringFunc)(const std::string& string);

//--------------------------------------------------------------------------------------------
std::string GenerateRandomString(const char *alphabet, int alphabetSize, int strLen);

//--------------------------------------------------------------------------------------------
void PadStrToMult(std::string &str, const char sym, int multLen);

//--------------------------------------------------------------------------------------------
bool Test(const std::string& inputStr, TransformStringFunc encodeFunc, TransformStringFunc decodeFunc);

//--------------------------------------------------------------------------------------------
#endif // UTILS_H