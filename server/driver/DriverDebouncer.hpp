/**
 * @brief
 * Non-blocking background debouncer for JugDriver
 * (NOTE: I ain't got no idea how this works exactly, Gemini did it TT)
**/

#pragma once

#include "common/headers.hpp"
#include "../../core/driver/JugDriver.hpp"

// Basic C++ headers
#include <thread>
#include <condition_variable>
#include <chrono>
#include <atomic>

namespace Driver {
    class DriverDebouncer {
        public:
            using JugDriver = Driver::JugDriver::JugDriver;

        private:
            std::mutex mtx;
            std::condition_variable cv;
            std::chrono::steady_clock::time_point triggerTime{};
            bool pending = false;
            std::atomic<bool> shutdown{false};
            std::thread worker;

            JugDriver* driverPtr = nullptr;

        public:
            std::atomic<bool> allowRuns{false};

            explicit DriverDebouncer(JugDriver& driver);
            ~DriverDebouncer();

            DriverDebouncer(const DriverDebouncer&) = delete;
            DriverDebouncer& operator=(const DriverDebouncer&) = delete;

            // Non-blocking: resets the 250ms quiet timer and wakes the worker if idle
            void trigger();
    };
}
