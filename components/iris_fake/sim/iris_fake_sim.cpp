#include <sim_i_hardware_model.hpp>
#include <boost/asio.hpp>
#include <fcntl.h>
#include <unistd.h>

namespace Nos3
{
// Python owns the pin state. This adapter forwards NOS3 commands to Python
// and reflects its response into the GPIO file shared with cFS.
class IrisFake : public SimIHardwareModel
{
public:
    explicit IrisFake(const boost::property_tree::ptree& config)
        : SimIHardwareModel(config) {}

    void command_callback(NosEngine::Common::Message msg) override
    {
        NosEngine::Common::DataBufferOverlay data(
            const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        boost::asio::io_service io;
        boost::asio::ip::tcp::socket socket(io);
        socket.connect({boost::asio::ip::address::from_string("127.0.0.1"), 12021});
        const std::string request = std::string(data.data) + "\n";
        boost::asio::write(socket, boost::asio::buffer(request));

        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, '\n');
        std::istream input(&response);
        std::string reply;
        std::getline(input, reply);
        if (reply == "0" || reply == "1")
        {
            // Overwrite one byte without truncating: readers always see a level.
            int fd = open("/tmp/gpio-fake/gpio1/value", O_WRONLY);
            if (fd >= 0)
            {
                reply = write(fd, reply.data(), 1) == 1 ? "OK" : "GPIO write failed";
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
