#ifndef DHT_ANALYZER_H
#define DHT_ANALYZER_H

#include <Analyzer.h>

#include "DHTSimulationDataGenerator.h"

// forward decls
class DHTAnalyzerSettings;
class DHTAnalyzerResults;

class ANALYZER_EXPORT DHTAnalyzer : public Analyzer2
{
    public:
        DHTAnalyzer();
        virtual ~DHTAnalyzer();

        void SetupResults() override;
        void WorkerThread() override;

        U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels ) override;
        U32 GetMinimumSampleRateHz() override;

        const char* GetAnalyzerName() const override;
        bool NeedsRerun() override;

    protected: //vars
        std::unique_ptr< DHTAnalyzerSettings > mSettings;
        std::unique_ptr< DHTAnalyzerResults > mResults;
        AnalyzerChannelData* mChannelData = nullptr;

        DHTSimulationDataGenerator mSimulationDataGenerator;
        bool mSimulationInitialized = false;

    private:

};

extern "C" {
    ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
    ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
    ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );
}

#endif // DHT_ANALYZER_H
