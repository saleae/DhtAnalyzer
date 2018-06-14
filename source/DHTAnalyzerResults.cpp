#include "DHTAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "DHTAnalyzer.h"
#include "DHTAnalyzerSettings.h"
#include "DHTAnalyzerHelpers.h"

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
    Frame frame = GetFrame( frame_index );

    // low 8 bits of mData2 are the frame type in the packet
    const DHTFrameType frameType = static_cast<DHTFrameType>(frame.mData2 & 0xff);
    switch (frameType) {
    case HOST_START_SIGNAL:
        AddResultString( "Host Start Signal" );
        AddResultString( "Start" );
        break;

    case SENSOR_RESPONSE_SIGNAL:
        AddResultString( "Sensor Response Signal ");
        AddResultString( "Response");
        break;

    case RH_DATA:
    {
        const double rh = frame.mData1 / 10.0;
        char buf[64];
        snprintf(buf, 64, "%3.1f%%", rh);
        AddResultString( "Humidity: ", buf);
        AddResultString( "H:", buf);
        break;
    }

    case TEMP_DATA:
    {
        const bool tempIsNegative = frame.mData1 & 0x8000;
        frame.mData1 &= 0x7fff; // clear the sign bit
        const double temp = tempIsNegative ? -(frame.mData1 / 10.0f) : (frame.mData1 / 10.0f);
        char buf[64];
        snprintf(buf, 64, "%3.1f", temp);
        AddResultString( "Temperature: ", buf);
        AddResultString( "T:", buf);
        break;
    };


    case CHECKSUM_DATA:
    {
        bool error = frame.HasFlag(DISPLAY_AS_ERROR_FLAG);
        char buf[64];
        U32 sum = frame.mData1;
        if (error) {
            // the analysis code saves the expected checksum into bits 15..8
            // of mData2, so we don't have to recompute it here
            const U8 expected = frame.mData2 >> 8;
            snprintf(buf, 64, "Expected: 0x%02x Read: 0x%02x", expected, sum);
            AddResultString("CRC Error. ", buf);
            AddResultString("Checksum ERROR");
        } else {
            snprintf(buf, 64, "0x%02x", sum);
            AddResultString("Checksum: ", buf);
            AddResultString(buf);
        }
        break;
    }

    } // of frame type switch
}

void DHTAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
    std::ofstream file_stream( file, std::ios::out );

    file_stream << "Time [s], Packet ID, Humidity, Temperature, CRC" << std::endl;

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    const U64 num_packets = GetNumPackets();

    for ( U64 i = 0; i < num_packets; i++ )
    {
        U64 firstFrame = 0, lastFrame = 0;
        GetFramesContainedInPacket(i, &firstFrame, &lastFrame);
        if ((lastFrame - firstFrame) != 4)
        {
            std::cerr << "DHT: incorrect number of frames in packet" << std::endl;
            continue;
        }

        const Frame hostStartFrame = GetFrame(firstFrame);
        const Frame rhFrame = GetFrame(firstFrame + 2);
        Frame tempFrame = GetFrame(firstFrame + 3);
        const Frame checkSumFrame = GetFrame(lastFrame);

        char time_str[128];
        AnalyzerHelpers::GetTimeString( hostStartFrame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

        const double rh = rhFrame.mData1 / 10.0;
        const bool tempIsNegative = tempFrame.mData1 & 0x8000;
        tempFrame.mData1 &= 0x7fff; // clear the sign bit
        const double temp = tempIsNegative ? -(tempFrame.mData1 / 10.0f) : (tempFrame.mData1 / 10.0f);

        file_stream << time_str << "," << i << "," << rh << ","
                    << temp << ","
                    << checkSumFrame.mData1 << std::endl;

        if ( UpdateExportProgressAndCheckForCancel( i, num_packets ) == true )
        {
            file_stream.close();
            return;
        }
    }

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
