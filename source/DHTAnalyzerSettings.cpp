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
    mInputChannelInterface->SetTitleAndTooltip( "Data", "Data" );
    mInputChannelInterface->SetChannel( mInputChannel );

    AddInterface( mInputChannelInterface.get() );

    mDeviceTypeInterface.reset( new AnalyzerSettingInterfaceNumberList );
    mDeviceTypeInterface->SetTitleAndTooltip( "Device", "" );
    mDeviceTypeInterface->AddNumber( DEVICE_DHT22, "DHT22", "");
    mDeviceTypeInterface->AddNumber( DEVICE_DHT11, "DHT11", "" );
    mDeviceTypeInterface->SetNumber( DEVICE_DHT22 );
    AddInterface( mDeviceTypeInterface.get() );

    mTemperatureUnitsInterface.reset( new AnalyzerSettingInterfaceNumberList );
    mTemperatureUnitsInterface->SetTitleAndTooltip( "Temperature Units", "" );
    mTemperatureUnitsInterface->AddNumber( UNITS_CELSIUS, "Celsius", "");
    mTemperatureUnitsInterface->AddNumber( UNITS_FARENHEIT, "Farenheit", "");
    mTemperatureUnitsInterface->AddNumber( UNITS_KELVIN, "Kelvin", "");
    mTemperatureUnitsInterface->SetNumber(UNITS_CELSIUS);

    AddInterface( mTemperatureUnitsInterface.get() );

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

    int unitsInt = static_cast<int>(mTemperatureUnitsInterface->GetNumber()); // explicit for MSVC
    mTemperatureUnits = static_cast<TemperatureUnits>(unitsInt);

    int deviceInt = static_cast<int>(mDeviceTypeInterface->GetNumber()); // explicit for MSVC
    mDeviceType = static_cast<DeviceType>(deviceInt);

    return true;
}

void DHTAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
    mDeviceTypeInterface->SetNumber(static_cast<int>(mDeviceType));
    mTemperatureUnitsInterface->SetNumber(static_cast<int>(mTemperatureUnits));
}

void DHTAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    U32 temp, device;
    text_archive >> mInputChannel >> temp >> device;
    mTemperatureUnits = static_cast<TemperatureUnits>(temp);
    mDeviceType = static_cast<DeviceType>(device);

    ClearChannels();
    AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, true );

    UpdateInterfacesFromSettings();
}

const char* DHTAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << mInputChannel
                 << static_cast<U32>(mTemperatureUnits)
                 << static_cast<U32>(mDeviceType);

    return SetReturnString( text_archive.GetString() );
}

