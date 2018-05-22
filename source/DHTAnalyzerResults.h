#ifndef DHT_ANALYZER_RESULTS
#define DHT_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class DHTAnalyzer;
class DHTAnalyzerSettings;

class DHTAnalyzerResults : public AnalyzerResults
{
    public:
        DHTAnalyzerResults( DHTAnalyzer* analyzer, DHTAnalyzerSettings* settings );
        virtual ~DHTAnalyzerResults();

        void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base ) override;
        void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id ) override;

        void GenerateFrameTabularText( U64 frame_index, DisplayBase display_base ) override;
        void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base ) override;
        void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base ) override;

    protected: //functions

    protected:  //vars
        DHTAnalyzerSettings* mSettings = nullptr;
        DHTAnalyzer* mAnalyzer = nullptr;
    private:

};

#endif // DHT_ANALYZER_RESULTS
