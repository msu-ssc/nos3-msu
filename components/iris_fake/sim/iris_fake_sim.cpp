#include <sim_i_hardware_model.hpp>
#include <fcntl.h>
#include <unistd.h>

namespace Nos3
{
// The launcher initializes GPIO1 low and shares its directory with cFS.
// No SPI, radio link, or dynamics provider is needed for this toy model.
class IrisFake : public SimIHardwareModel
{
public:
    explicit IrisFake(const boost::property_tree::ptree& config)
        : SimIHardwareModel(config) {}

    void command_callback(NosEngine::Common::Message msg) override
    {
        NosEngine::Common::DataBufferOverlay data(
            const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        const std::string command = data.data;
        std::string reply = "Expected GPO1=0 or GPO1=1";
        if (command == "GPO1=0" || command == "GPO1=1")
        {
            // Overwrite one byte without truncating: readers always see a level.
            int fd = open("/tmp/gpio-fake/gpio1/value", O_WRONLY);
            if (fd >= 0)
            {
                reply = write(fd, &command.back(), 1) == 1 ? "OK" : "GPIO write failed";
                close(fd);
            }
            else
                reply = "GPIO1 missing: check the launcher's shared directory";
        }
        _command_node->send_reply_message_async(msg, reply.size(), reply.c_str());
    }
};
REGISTER_HARDWARE_MODEL(IrisFake, "IRIS_FAKE");
}
