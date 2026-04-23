/**
 * @brief
 * Manage document objects and content updates
**/

#pragma once

// Common headers
#include "common/headers.hpp"

// Session
#include "../../core/session/session.hpp"

// Basic C++ headers
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

namespace Session {
    class SessionDebouncer {
        private:
            std::mutex mtx;
            std::condition_variable cv;
            std::atomic<bool> shutdown{false};
            std::chrono::steady_clock::time_point triggerTime;
            bool pending = false;
            std::thread worker;

            Session* sessionPtr;

        public:
            SessionDebouncer(Session& session) ;
            ~SessionDebouncer() ;
            void trigger() ;
    };
}