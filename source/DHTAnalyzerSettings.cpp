#include "DHTAnalyzerSettings.h"

#include <cassert>

#include <AnalyzerHelpers.h>

const char* DEFAULT_CHANNEL_NAME = "DHT";

double operator "" _ns( unsigned long long x )
{
    return x * 1e-9;
}

double operator "" _us( unsigned long long x )
{
    return x * 1e-6;
}

DHTAnalyzerSettings::DHTAnalyzerSettings()
{
    mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mInputChannelInterface->SetTitleAndTooltip( "DHT Channel", "DHT" );
    mInputChannelInterface->SetChannel( mInputChannel );

    AddInterface( mInputChannelInterface.get() );

    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, false );
}

DHTAnalyzerSettings::~DHTAnalyzerSettings()
{
}

bool DHTAnalyzerSettings::SetSettingsFromInterfaces()
{
    mInputChannel = mInputChannelInterface->GetChannel();

    ClearChannels();
    AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, true );

    return true;
}

void DHTAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
}

void DHTAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    text_archive >> mInputChannel;

    ClearChannels();
    AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, true );

    UpdateInterfacesFromSettings();
}

const char* DHTAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mInputChannel;

    return SetReturnString( text_archive.GetString() );
}

