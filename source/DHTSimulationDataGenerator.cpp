#include "DHTSimulationDataGenerator.h"

#include <cmath>
#include <iostream>
#include <cassert>

#include "DHTAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

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


DHTSimulationDataGenerator::DHTSimulationDataGenerator()
{
}

DHTSimulationDataGenerator::~DHTSimulationDataGenerator()
{
}

void DHTSimulationDataGenerator::Initialize( U32 simulation_sample_rate, DHTAnalyzerSettings* settings )
{
    mSettings = settings;

}

U32 DHTSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
    #if 0
    U64 adjusted_largest_sample_requested =
        AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while ( mSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
       
    }
#endif
    *simulation_channel = &mSimulationData;
    return 1;
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
    WriteUIntData(0, 0);
    WriteUIntData(0, 0);

    // sensor end pulse - how long?
    const U32 sensorEndSendSamples = mClockGenerator.AdvanceByTimeS( 80_us );
    mSimulationData.Transition(); // go low
    mSimulationData.Advance( sensorEndSendSamples );
    mSimulationData.Transition(); // go high
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
