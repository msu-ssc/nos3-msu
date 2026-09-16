#ifndef NOS3_SAMPLE_RADIO_SHMEM_DATA_PROVIDER_HPP
#define NOS3_SAMPLE_RADIO_SHMEM_DATA_PROVIDER_HPP

#include <boost/property_tree/ptree.hpp>
#include <ItcLogger/Logger.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <sample_radio_data_point.hpp>
#include <sim_i_data_provider.hpp>
#include <blackboard_data.hpp>

namespace Nos3
{
    namespace bip = boost::interprocess;

    class Sample_radioShmemDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        Sample_radioShmemDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~Sample_radioShmemDataProvider(void) {};
        Sample_radioShmemDataProvider& operator=(const Sample_radioShmemDataProvider&) {return *this;};

        bip::mapped_region _shm_region;
        BlackboardData*    _blackboard_data;
    };
}

#endif
