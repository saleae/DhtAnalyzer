#ifndef DHT_SIMULATION_DATA_GENERATOR
#define DHT_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>
#include <string>

class DHTAnalyzerSettings;

class DHTSimulationDataGenerator
{
    public:
        DHTSimulationDataGenerator();
        ~DHTSimulationDataGenerator();

        void Initialize( U32 simulation_sample_rate, DHTAnalyzerSettings* settings );
        U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

    protected:
        DHTAnalyzerSettings* mSettings;
        U32 mSimulationSampleRateHz;

    protected:
       

        ClockGenerator mClockGenerator;
        SimulationChannelDescriptor mSimulationData;

private:
        void WriteBit(BitState b);
        void WriteUIntData(U16 data, U8 bit_count);
        void GeneratePacket();
};

#endif // DHT_SIMULATION_DATA_GENERATOR
