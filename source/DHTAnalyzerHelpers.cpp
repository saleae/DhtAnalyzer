#include "DHTAnalyzerHelpers.h"

U8 ComputeCheckSum(U16 rhValue, U16 tempValue)
{
    U8 result = 0;
    U8* tBytes = reinterpret_cast<U8*>(&tempValue);
    U8* hBytes = reinterpret_cast<U8*>(&rhValue);
    result += hBytes[1] + hBytes[1];
    result += tBytes[0] + tBytes[1];
    return result;
}

