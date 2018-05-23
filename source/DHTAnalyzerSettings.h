#ifndef DHT_ANALYZER_SETTINGS
#define DHT_ANALYZER_SETTINGS

#include <vector>

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DHTAnalyzerSettings : public AnalyzerSettings
{
public:
    DHTAnalyzerSettings();
    virtual ~DHTAnalyzerSettings();

    // delete all the copy/move operators for rule-of-5 compliance
    DHTAnalyzerSettings( const DHTAnalyzerSettings& ) = delete;
    DHTAnalyzerSettings( DHTAnalyzerSettings&& ) = delete;
    const DHTAnalyzerSettings& operator=( const DHTAnalyzerSettings& ) = delete;
    DHTAnalyzerSettings& operator=( DHTAnalyzerSettings&& ) = delete;


    bool SetSettingsFromInterfaces() override;
    void UpdateInterfacesFromSettings();
    void LoadSettings( const char* settings ) override;
    const char* SaveSettings() override;

    Channel mInputChannel = UNDEFINED_CHANNEL;

    enum DeviceType
    {
        DEVICE_DHT22 = 0,
        DEVICE_DHT11
    };

    enum TemperatureUnits
    {
        UNITS_CELSIUS = 0,
        UNITS_FARENHEIT,
        UNITS_KELVIN
    };

    TemperatureUnits GetTemperatureUnits() const
    { return mTemperatureUnits; }

    DeviceType GetDeviceType() const
    { return mDeviceType; }

protected:
    void InitControllerData();

    std::unique_ptr< AnalyzerSettingInterfaceChannel > mInputChannelInterface;
    std::unique_ptr< AnalyzerSettingInterfaceNumberList > mDeviceTypeInterface;
    std::unique_ptr< AnalyzerSettingInterfaceNumberList > mTemperatureUnitsInterface;

    DeviceType mDeviceType = DEVICE_DHT22;
    TemperatureUnits mTemperatureUnits = UNITS_CELSIUS;
};

#endif // DHT_ANALYZER_SETTINGS
