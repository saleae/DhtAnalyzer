#include "DHTAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "DHTAnalyzer.h"
#include "DHTAnalyzerSettings.h"

#include <iostream>
#include <fstream>

DHTAnalyzerResults::DHTAnalyzerResults( DHTAnalyzer* analyzer, DHTAnalyzerSettings* settings )
    :   AnalyzerResults(),
        mSettings( settings ),
        mAnalyzer( analyzer )
{
}

DHTAnalyzerResults::~DHTAnalyzerResults()
{
}

void DHTAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
    ClearResultStrings();
   
    //AddResultString( webBuf );
}

void DHTAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
    std::ofstream file_stream( file, std::ios::out );
#if 0
    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    file_stream << "Time [s], Packet ID, LED Index, Red, Green, Blue, Web-CSS" << std::endl;

    const U64 num_frames = GetNumFrames();

    for ( U64 i = 0; i < num_frames; i++ )
    {
        const Frame frame = GetFrame( i );
        U64 packetId = GetPacketContainingFrameSequential( num_frames );

        if ( packetId == INVALID_RESULT_INDEX )
        {
            packetId = -1;
        }

        char time_str[128];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

        RGBValue rgb = RGBValue::CreateFromU64( frame.mData1 );

        // RGB numerical value representation
        const size_t bufSize = 16;
        char rs[bufSize], gs[bufSize], bs[bufSize];
        GenerateRGBStrings( rgb, display_base, bufSize, rs, gs, bs );

        // CSS representation
        U8 webColor[3];
        rgb.ConvertTo8Bit( mSettings->BitSize(), webColor );
        char webBuf[8];
        ::snprintf( webBuf, sizeof( webBuf ), "#%02x%02x%02x", webColor[0], webColor[1], webColor[2] );

        file_stream << time_str << "," << packetId << "," << frame.mData2 << ","
                    << rs << ","
                    << gs << ","
                    << bs << ","
                    << webBuf << std::endl;

        if ( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
        {
            file_stream.close();
            return;
        }
    }
#endif
    file_stream.close();
}

void DHTAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH

#endif
}

void DHTAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
    //not supported

}

void DHTAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
    //not supported
}
