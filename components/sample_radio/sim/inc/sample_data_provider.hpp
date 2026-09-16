#ifndef NOS3_SAMPLE_RADIODATAPROVIDER_HPP
#define NOS3_SAMPLE_RADIODATAPROVIDER_HPP

#include <boost/property_tree/xml_parser.hpp>
#include <ItcLogger/Logger.hpp>
#include <sample_radio_data_point.hpp>
#include <sim_i_data_provider.hpp>

namespace Nos3
{
    class Sample_radioDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        Sample_radioDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Sample_radioDataProvider(void) {};
        Sample_radioDataProvider& operator=(const Sample_radioDataProvider&) {return *this;};

        mutable double _request_count;
    };
}

#endif
