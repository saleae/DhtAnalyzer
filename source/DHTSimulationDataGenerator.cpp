#include "DHTSimulationDataGenerator.h"

#include <cmath>
#include <iostream>
#include <cassert>

#include "DHTAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

namespace {

double operator "" _ns( unsigned long long x )
{
    return x * 1e-9;
}

double operator "" _us( unsigned long long x )
{
    return x * 1e-6;
}

double operator "" _ms( unsigned long long x )
{
    return x * 1e-3;
}

} // of anonymous namespace

DHTSimulationDataGenerator::DHTSimulationDataGenerator()
{
}

DHTSimulationDataGenerator::~DHTSimulationDataGenerator()
{
}

void DHTSimulationDataGenerator::Initialize( U32 simulation_sample_rate, DHTAnalyzerSettings* settings )
{
    // Initialize the random number generator with a literal seed to obtain repeatability
    // Change this for srand(time(NULL)) for "truly" random sequences
    // NOTICE rand() an srand() are *not* thread safe
    srand( 42 );

    mSettings = settings;
    mSimulationSampleRateHz = simulation_sample_rate;

    const double clockFrequencyUnused = 1.0;
    mClockGenerator.Init( clockFrequencyUnused, mSimulationSampleRateHz );

    mSimulationData.SetChannel( mSettings->mInputChannel );
    mSimulationData.SetSampleRate( simulation_sample_rate );
    mSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 DHTSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested,
                                                        U32 sample_rate,
                                                        SimulationChannelDescriptor** simulation_channel )
{
    U64 adjusted_largest_sample_requested =
        AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while ( mSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        // small dleay at the start, so we don't begin the host start pulse
        // on sample zero.
        const U32 startDelaySamples = mClockGenerator.AdvanceByTimeS( 10_ms );
        mSimulationData.Advance(startDelaySamples);

        GeneratePacket();

        // delay time
        // the spec PDF says 'the interval of whole process must beyond 2 seconds.'
        const U32 delaySamples = mClockGenerator.AdvanceByTimeS( 2.0 );
        mSimulationData.Advance(delaySamples);
    }

    *simulation_channel = &mSimulationData;
    return 1;
}

U8 ComputeCheckSum(U16 temp, U16 humidity)
{
    U8 result = 0;
    U8* tBytes = reinterpret_cast<U8*>(&temp);
    U8* hBytes = reinterpret_cast<U8*>(&humidity);
    result += tBytes[0] + tBytes[1];
    result += hBytes[1] + hBytes[1];
    return result;
}

void DHTSimulationDataGenerator::GeneratePacket()
{
    // master/host side
    assert( mSimulationData.GetCurrentBitState() == BIT_HIGH );
    const U32 startSamples = mClockGenerator.AdvanceByTimeS( 2_ms );
    const U32 waitSamples = mClockGenerator.AdvanceByTimeS( 30_us );

    mSimulationData.Transition(); // go low
    mSimulationData.Advance( startSamples );
    mSimulationData.Transition(); // go high
    mSimulationData.Advance( waitSamples );

    // write sensor start pulse
    const U32 sensorStartSendSamples = mClockGenerator.AdvanceByTimeS( 80_us );
    const U32 sensorWaitStartSamples = mClockGenerator.AdvanceByTimeS( 80_us );

    mSimulationData.Transition(); // go low
    mSimulationData.Advance( sensorStartSendSamples );
    mSimulationData.Transition(); // go high
    mSimulationData.Advance( sensorWaitStartSamples );

    // write data bits
    U16 humidityData = rand() % 1000;
    U16 tempData = rand() % 500;

    // 50% chance we make the temperature negative by setting the top bit
    if (rand() % 2) {
        tempData |= 0x8000;
    }

    const U16 checkSum = ComputeCheckSum(humidityData, tempData);

    WriteUIntData(humidityData, 16);
    WriteUIntData(tempData, 16);
    WriteUIntData(checkSum, 8);

    // sensor end pulse - how long?
    const U32 sensorEndSendSamples = mClockGenerator.AdvanceByTimeS( 80_us );
    mSimulationData.Transition(); // go low
    mSimulationData.Advance( sensorEndSendSamples );
    mSimulationData.Transition(); // go high

    assert( mSimulationData.GetCurrentBitState() == BIT_HIGH );
}

void DHTSimulationDataGenerator::WriteUIntData( U16 data, U8 bit_count )
{
    BitExtractor extractor( data, AnalyzerEnums::MsbFirst, bit_count );

    for ( U32 bit = 0; bit < bit_count; ++bit )
    {
        WriteBit( extractor.GetNextBit() );
    }
}

void DHTSimulationDataGenerator::WriteBit( bool b )
{
    assert( mSimulationData.GetCurrentBitState() == BIT_HIGH );

    const U32 lowSamples = mClockGenerator.AdvanceByTimeS( 50_us );
    const double highTime = (b ? 70_us : 27_us);
    const U32 highSamples = mClockGenerator.AdvanceByTimeS( highTime );

    mSimulationData.Transition(); // go low
    mSimulationData.Advance( lowSamples );
    mSimulationData.Transition(); // go high
    mSimulationData.Advance( highSamples );
}
