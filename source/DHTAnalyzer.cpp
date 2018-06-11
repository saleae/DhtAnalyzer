#include "DHTAnalyzer.h"

#include "DHTAnalyzerSettings.h"
#include "DHTAnalyzerResults.h"
#include "DHTAnalyzerHelpers.h"

#include <AnalyzerChannelData.h>

#include <iostream>

#define DHT_LOGGING 1

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

DHTAnalyzer::DHTAnalyzer()
    :   Analyzer2()
    ,   mSettings( new DHTAnalyzerSettings )
{
    SetAnalyzerSettings( mSettings.get() );
}

DHTAnalyzer::~DHTAnalyzer()
{
    KillThread();
}

void DHTAnalyzer::SetupResults()
{
     mResults.reset( new DHTAnalyzerResults( this, mSettings.get() ) );
     SetAnalyzerResults( mResults.get() );
     mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void DHTAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();
    mChannelData = GetAnalyzerChannelData( mSettings->mInputChannel );

    for ( ; ; )
    {
        AdvanceToHostStartPulse();
        // channel data is now on rising edge of host request pulse

        ReadPacket();
    }

    mResults->CommitResults();

}

void DHTAnalyzer::AdvanceToHostStartPulse()
{
    if ( mChannelData->GetBitState() == BIT_HIGH )
    {   // look for a falling edge, indicating beginning of host request
        mChannelData->AdvanceToNextEdge();
    }

    for ( ; ; )
    {
        const U64 lowTransition = mChannelData->GetSampleNumber();
        const U64 highTransition = mChannelData->GetSampleOfNextEdge();
        double lowTimeSec = ( highTransition - lowTransition ) / mSampleRateHz;

        if ((lowTimeSec > 1_ms) && (lowTimeSec < 10_ms))
        {
            mChannelData->AdvanceToAbsPosition( highTransition );
            std::cout << "found start pulse" << std::endl;
            return;
        }

        // advance past the rising edge, to the next falling edge,
        // which is our next candidate for the beginning of hos rstart pulse
        mChannelData->AdvanceToAbsPosition( highTransition );
        mChannelData->AdvanceToNextEdge();
    }
}

auto DHTAnalyzer::ReadDataBit() -> ReadResult
{
    ReadResult result{BIT_LOW, false};

    assert(mChannelData->GetBitState() == BIT_LOW);
    const double dataMaxLowSamples = 60_us * mSampleRateHz;

    if ( !mChannelData->WouldAdvancingCauseTransition( dataMaxLowSamples ) )
    {
#if defined(DHT_LOGGING)
        std::cout << "data bit low pulse too long" << std::endl;
#endif
        return result;
    }

    double dataLowTimeSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber()) / mSampleRateHz;
    if (dataLowTimeSec < 40_us)
    {
#if defined(DHT_LOGGING)
        std::cout << "data bit low pulse too short" << std::endl;
#endif
        return result;
    }

    mChannelData->AdvanceToNextEdge();

    const double dataMaxHighSamples = 80_us * mSampleRateHz;
    if ( !mChannelData->WouldAdvancingCauseTransition( dataMaxHighSamples ) )
    {
#if defined(DHT_LOGGING)
        std::cout << "data bit high pulse too long" << std::endl;
#endif
        return result;
    }

    double dataHighTimeSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber()) / mSampleRateHz;
    mChannelData->AdvanceToNextEdge();

    if (dataHighTimeSec < 20_us)
    {
#if defined(DHT_LOGGING)
        std::cout << "data bit high pulse too short" << std::endl;
#endif
        return result;
    }

    // time is ambiguously between the low duration and high duration
    if ((dataHighTimeSec > 40_us) && (dataHighTimeSec < 60_us))
    {
#if defined(DHT_LOGGING)
        std::cout << "data bit high pulse of invalid length:" << dataHighTimeSec << std::endl;
#endif
        return result;
    }

    assert(mChannelData->GetBitState() == BIT_LOW);
    result.bit = dataHighTimeSec < 50_us ? BIT_LOW : BIT_HIGH;
    result.valid = true;
    return result;
}

void DHTAnalyzer::ReadPacket()
{
    U64 packetBeginSample = mChannelData->GetSampleNumber();

    const double hostMaxWaitSamples = 40_us * mSampleRateHz;
    if ( !mChannelData->WouldAdvancingCauseTransition( hostMaxWaitSamples ) )
    {
        // we didn't see sensor start pulse within the require time, give up
#if defined(DHT_LOGGING)
        std::cout << "No sensor start pulse within 40 usec of host request" << std::endl;
#endif
        return;
    }

    double hostWaitTimeSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber() ) / mSampleRateHz;
    if (hostWaitTimeSec < 20_us)
    {
        // sensor started replying too early
        // should we accept this or bail out? Bailing out for now
#if defined(DHT_LOGGING)
        std::cout << "Sensor started response within 20 usec of host request" << std::endl;
#endif
        return;
    }

    // advance to the edge where the sensor pulls low
    mChannelData->AdvanceToNextEdge();

    // measure sensor low and high start pulses - both should be 80_us approx
    // no tolerance value supplied in the data-sheet!
    const double maxSensorStartSamples = 90_us * mSampleRateHz;
    if ( !mChannelData->WouldAdvancingCauseTransition( maxSensorStartSamples ) )
    {
#if defined(DHT_LOGGING)
        std::cout << "Sensor start pulse longer than 80 usec" << std::endl;
#endif
        return;
    }

    double sensorStartLowTimeSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber()) / mSampleRateHz;
    if (sensorStartLowTimeSec < 70_us)
    {
#if defined(DHT_LOGGING)
        std::cout << "Sensor start pulse too short:" << sensorStartLowTimeSec << std::endl;
#endif
        return;
    }

    // advance to the edge where the sensor pulls low, and repeat the preceeding
    // steps
    mChannelData->AdvanceToNextEdge();

    if ( !mChannelData->WouldAdvancingCauseTransition( maxSensorStartSamples ) )
    {
#if defined(DHT_LOGGING)
        std::cout << "Sensor preparation pulse longer than 80 usec" << std::endl;
#endif
        return;
    }

    double sensorStartHighTimeSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber()) / mSampleRateHz;
    if (sensorStartHighTimeSec < 70_us)
    {
#if defined(DHT_LOGGING)
        std::cout << "Sensor preparation pulse too short:" << sensorStartHighTimeSec << std::endl;
#endif
        return;
    }

    std::cout << "Sensor did sync to first data bit"  << std::endl;

    // advance to the first data bit
    mChannelData->AdvanceToNextEdge();
    U64 rhData, tempData, checkSumBits;

    DataBuilder rhBuilder;
    rhBuilder.Reset(&rhData, AnalyzerEnums::MsbFirst, 16);
    for (int i=0; i<16; ++i) {
        auto r = ReadDataBit();
        if (!r.valid)
        {
            return;
        }
        rhBuilder.AddBit(r.bit);
    }

    DataBuilder tempBuilder;
    tempBuilder.Reset(&tempData, AnalyzerEnums::MsbFirst, 16);
    for (int i=0; i<16; ++i) {
        auto r = ReadDataBit();
        if (!r.valid)
        {
            return;
        }

        tempBuilder.AddBit(r.bit);
    }

    DataBuilder checkSumBuilder;
    checkSumBuilder.Reset(&checkSumBits, AnalyzerEnums::MsbFirst, 8);
    for (int i=0; i<8; ++i) {
        auto r = ReadDataBit();
        if (!r.valid)
        {
            return;
        }

        checkSumBuilder.AddBit(r.bit);
    }

    U8 computedSum = ComputeCheckSum(rhData, tempData);
    if (computedSum != checkSumBits)
    {
        std::cerr << "checksum error" << std::endl;
        return;
    }

    bool tempIsNegative = tempData & 0x8000;
    tempData &= 0x7fff; // clear the sign bit

#if defined(DHT_LOGGING)
    std::cout << "Sensor did read packet"  << std::endl;
    std::cout << "RH:" << rhData / 10.0 << std::endl;
    std::cout << "temp " << tempData / 10.0 << " sign:" << tempIsNegative << std::endl;
#endif

    Frame frame;
    frame.mFlags = 0;
    frame.mStartingSampleInclusive = packetBeginSample;
    frame.mEndingSampleInclusive = mChannelData->GetSampleNumber();
    frame.mData1 = rhData;
    frame.mData2 = tempData;
    mResults->AddFrame( frame );

    mResults->CommitResults();
    ReportProgress( mChannelData->GetSampleNumber() );
}

bool DHTAnalyzer::NeedsRerun()
{
    return false;
}

U32 DHTAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
    if ( mSimulationInitialized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitialized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 DHTAnalyzer::GetMinimumSampleRateHz()
{
    return 12 * 100000; // 12Mhz minimum sample rate
}

const char* DHTAnalyzer::GetAnalyzerName() const
{
    return "DHT22/DHT11";
}

const char* GetAnalyzerName()
{
    return "DHT22/DHT11";
}

Analyzer* CreateAnalyzer()
{
    return new DHTAnalyzer;
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
