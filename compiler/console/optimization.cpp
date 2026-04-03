/**
 * @brief
 * Include all used report headers
**/

#include "optimization.hpp"

namespace Console {
    namespace Optimization {
        void optimize() {
            // Flush all channels
            std::cout.flush();
            std::clog.flush();
            std::fflush(stdout);
            std::fflush(stderr);

            // Change flushing behaviour
            std::cout << std::nounitbuf;
            std::clog << std::nounitbuf;

            // Sever the input channel
            std::cin.tie(nullptr);

            // Control buffer size
            static std::vector<char> outBuffer(512);
            static std::vector<char> errBuffer(512);
            std::setvbuf(stdout, outBuffer.data(), _IOFBF, outBuffer.size());
            std::setvbuf(stderr, errBuffer.data(), _IOFBF, errBuffer.size());
        }
        void reverse() {
            // Flush all channels
            std::cout.flush();
            std::clog.flush();
            std::fflush(stdout);
            std::fflush(stderr);

            // Reconnect input channel
            std::cin.tie(&std::cout);

            // Reset buffer size
            std::setvbuf(stderr, nullptr, _IONBF, 0);
            std::setvbuf(stdout, nullptr, _IONBF, 0);
        }
        void safetyCheck() {
            // Flush all channels
            std::cout.flush();
            std::clog.flush();
            std::fflush(stdout);
            std::fflush(stderr);
        }
    }
}
