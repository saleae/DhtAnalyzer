#include "DHTAnalyzer.h"
#include "DHTAnalyzerSettings.h"
#include "DHTAnalyzerResults.h"

#include <AnalyzerChannelData.h>

#include <iostream>


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
    #if 0
    mSampleRateHz = GetSampleRate();
    mHalfSampleWidth = 0.5 / mSampleRateHz;
    mChannelData = GetAnalyzerChannelData( mSettings->mInputChannel );

    // cache this value here to avoid recomputing this every bit-read
    if ( mSettings->IsHighSpeedSupported() )
    {
        mMinimumLowDurationSec = std::min( mSettings->DataTiming( BIT_LOW, true ).mNegativeTiming.mMinimumSec,
                                           mSettings->DataTiming( BIT_HIGH, true ).mNegativeTiming.mMinimumSec );
    }
    else
    {
        mMinimumLowDurationSec = std::min( mSettings->DataTiming( BIT_LOW ).mNegativeTiming.mMinimumSec,
                                           mSettings->DataTiming( BIT_HIGH ).mNegativeTiming.mMinimumSec );
    }

    mMinimumLowDurationSec-= mHalfSampleWidth;

    bool isResyncNeeded = true;

    for ( ; ; )
    {
        if ( isResyncNeeded )
        {
            SynchronizeToReset();
            isResyncNeeded = false;
        }

        mFirstBitAfterReset = true;
        U32 frameInPacketIndex = 0;
        mResults->CommitPacketAndStartNewPacket();

        // data word reading loop
        for ( ; ; )
        {
            auto result = ReadRGBTriple();

            if ( result.mValid )
            {
                Frame frame;
                frame.mFlags = 0;
                frame.mStartingSampleInclusive = result.mValueBeginSample;
                frame.mEndingSampleInclusive = result.mValueEndSample;
                frame.mData1 = result.mRGB.ConvertToU64();
                frame.mData2 = frameInPacketIndex++;
                mResults->AddFrame( frame );
                mResults->CommitResults();
            }
            else
            {
#if defined(LED_LOGGING)
                std::cerr << "failed to read frame at:" << (frameInPacketIndex) << std::endl;
#endif
                // something error occurred, let's resynchronise
                isResyncNeeded = true;
            }

            if ( isResyncNeeded || result.mIsReset )
            {
                break;
            }
        }

        mResults->CommitResults();
        ReportProgress( mChannelData->GetSampleNumber() );
    }
    #endif
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
