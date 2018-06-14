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
            mLastHostStartPulse = lowTimeSec; // save this, for frame generation
            mChannelData->AdvanceToAbsPosition( highTransition );
            return;
        }

        // advance past the rising edge, to the next falling edge,
        // which is our next candidate for the beginning of host start pulse
        mChannelData->AdvanceToAbsPosition( highTransition );
        mChannelData->AdvanceToNextEdge();
    }
}

bool DHTAnalyzer::CheckForPulse(double minDurationSec, double maxDurationSec)
{
    const U32 maxSamples = static_cast<U32>(maxDurationSec * mSampleRateHz);

    if ( !mChannelData->WouldAdvancingCauseTransition( maxSamples ) )
    {
#if defined(DHT_LOGGING)
        std::cout << "CheckForPulse: transition not found within time:" << maxDurationSec << std::endl;
#endif
        return false;
    }

    double pulseDurationSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber()) / mSampleRateHz;
    if (pulseDurationSec < minDurationSec)
    {
#if defined(DHT_LOGGING)
        std::cout << "CheckForPulse: pulse duration too short:"
                  << pulseDurationSec << ", need at least "
                  << minDurationSec << std::endl;
#endif
        return false;
    }

    return true;
}

auto DHTAnalyzer::ReadDataBit() -> ReadResult
{
    ReadResult result{BIT_LOW, false};

    // low pulse always 50_usec
    assert(mChannelData->GetBitState() == BIT_LOW);
    if (!CheckForPulse(40_us, 60_us)) {
        return result;
    }

    mChannelData->AdvanceToNextEdge();
    if (!CheckForPulse(20_us, 80_us)) {
        return result;
    }

    double dataHighTimeSec = (mChannelData->GetSampleOfNextEdge() - mChannelData->GetSampleNumber()) / mSampleRateHz;
    mChannelData->AdvanceToNextEdge();

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

    mResults->CommitPacketAndStartNewPacket();
    Frame hostBeginFrame;
    hostBeginFrame.mStartingSampleInclusive = mLastHostStartPulse;
    hostBeginFrame.mData2 = HOST_START_SIGNAL;

    // host waiting for sensor to start output
    if (!CheckForPulse(20_us, 40_us)) {
#if defined(DHT_LOGGING)
        std::cout << "Host wait pulse out of tolerance" << std::endl;
#endif
        return;
    }

    // advance to the edge where the sensor pulls low
    mChannelData->AdvanceToNextEdge();
    hostBeginFrame.mEndingSampleInclusive = mChannelData->GetSampleNumber() - 1;
    mResults->AddFrame( hostBeginFrame );

    // measure sensor low and high start pulses - both should be 80_us approx
    // no tolerance value supplied in the data-sheet!

    Frame sensorResponseFrame;
    sensorResponseFrame.mStartingSampleInclusive = mChannelData->GetSampleNumber();
    sensorResponseFrame.mData2 = SENSOR_RESPONSE_SIGNAL;

    if (!CheckForPulse(70_us, 90_us)) {
#if defined(DHT_LOGGING)
        std::cout << "Sensor start pulse out of tolerance" << std::endl;
#endif
        return;
    }

    // advance to the edge where the sensor goes high
    mChannelData->AdvanceToNextEdge();

    if (!CheckForPulse(70_us, 90_us)) {
#if defined(DHT_LOGGING)
        std::cout << "Sensor wait pulse out of tolerance" << std::endl;
#endif
        return;
    }

    // advance to the first data bit
    mChannelData->AdvanceToNextEdge();
    sensorResponseFrame.mEndingSampleInclusive = mChannelData->GetSampleNumber() - 1;
    mResults->AddFrame( sensorResponseFrame );

// start with the data bits
    Frame relHumidityFrame,
            tempFrame,
            checkSumFrame;
    relHumidityFrame.mData2 = RH_DATA;
    tempFrame.mData2 = TEMP_DATA;
    checkSumFrame.mData2 = CHECKSUM_DATA;

    relHumidityFrame.mStartingSampleInclusive = mChannelData->GetSampleNumber();

    DataBuilder rhBuilder;
    rhBuilder.Reset(&relHumidityFrame.mData1, AnalyzerEnums::MsbFirst, 16);
    for (int i=0; i<16; ++i) {
        auto r = ReadDataBit();
        if (!r.valid)
        {
            return;
        }
        rhBuilder.AddBit(r.bit);
    }

    relHumidityFrame.mEndingSampleInclusive = mChannelData->GetSampleNumber() - 1;
    tempFrame.mStartingSampleInclusive = mChannelData->GetSampleNumber();

    DataBuilder tempBuilder;
    tempBuilder.Reset(&tempFrame.mData1, AnalyzerEnums::MsbFirst, 16);
    for (int i=0; i<16; ++i) {
        auto r = ReadDataBit();
        if (!r.valid)
        {
            return;
        }

        tempBuilder.AddBit(r.bit);
    }

    tempFrame.mEndingSampleInclusive = mChannelData->GetSampleNumber() - 1;
    checkSumFrame.mStartingSampleInclusive = mChannelData->GetSampleNumber();

    DataBuilder checkSumBuilder;
    checkSumBuilder.Reset(&checkSumFrame.mData1, AnalyzerEnums::MsbFirst, 8);
    for (int i=0; i<8; ++i) {
        auto r = ReadDataBit();
        if (!r.valid)
        {
            return;
        }

        checkSumBuilder.AddBit(r.bit);
    }

    checkSumFrame.mEndingSampleInclusive = mChannelData->GetSampleNumber() - 1;

    U8 computedSum = ComputeCheckSum(relHumidityFrame.mData1, tempFrame.mData1);
    if (computedSum != checkSumFrame.mData1)
    {
        checkSumFrame.mFlags |= DISPLAY_AS_ERROR_FLAG;
        // store the expected value in mData2, so it's available to the
        // results code easily.
        checkSumFrame.mData2 |= (computedSum << 8);
    } else {
        checkSumFrame.mFlags = 0;
    }

    mResults->AddFrame(relHumidityFrame);
    mResults->AddFrame(tempFrame);
    mResults->AddFrame(checkSumFrame);

    mResults->CommitResults();
    mResults->CommitPacketAndStartNewPacket();

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
