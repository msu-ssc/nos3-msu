#include <sample_radio_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(Sample_radioDataProvider,"SAMPLE_RADIO_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    Sample_radioDataProvider::Sample_radioDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("Sample_radioDataProvider::Sample_radioDataProvider:  Constructor executed");
        _request_count = 0;
    }

    boost::shared_ptr<SimIDataPoint> Sample_radioDataProvider::get_data_point(void) const
    {
        sim_logger->trace("Sample_radioDataProvider::get_data_point:  Executed");

        /* Prepare the provider data */
        _request_count++;

        /* Request a data point */
        SimIDataPoint *dp = new Sample_radioDataPoint(_request_count);

        /* Return the data point */
        return boost::shared_ptr<SimIDataPoint>(dp);
    }
}
