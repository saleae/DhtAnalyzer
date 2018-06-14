#ifndef DHT_ANALYZER_HELPERS_H
#define DHT_ANALYZER_HELPERS_H

#include <AnalyzerTypes.h>

U8 ComputeCheckSum(U16 rhValue, U16 tempValue);

enum DHTFrameType
{
    HOST_START_SIGNAL = 0,
    SENSOR_RESPONSE_SIGNAL,
    RH_DATA,
    TEMP_DATA,
    CHECKSUM_DATA
};

#endif
