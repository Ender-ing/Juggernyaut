/**
 * @brief
 * Manage document objects and content updates
**/

#include "SessionDebouncer.hpp"

// Full disclosure: I barely have any idea how this works...

namespace Session {
    SessionDebouncer::SessionDebouncer(Session& session)
        : sessionPtr(&session) {

        const auto delay = std::chrono::milliseconds(50);
        worker = std::thread([this, &delay]() {
            while (!shutdown) {
                std::unique_lock<std::mutex> lock(mtx);

                // Wait until a trigger is pending or we shut down
                cv.wait(lock, [this] { return pending || shutdown; });
                if (shutdown) break;

                // Wait until the debounce delay has passed since the LAST trigger
                auto now = std::chrono::steady_clock::now();
                do {
                    cv.wait_until(lock, triggerTime);
                    now = std::chrono::steady_clock::now();
                    if (shutdown) return;
                    if (!allowRuns && now >= triggerTime - delay) {
                        triggerTime += delay;
                    }
                } while (now < triggerTime);

                // If we get here, the timer expired without being interrupted!
                pending = false;
                lock.unlock(); // Unlock before heavy lifting

                // Run the session on this background thread
                initiate(*sessionPtr);
            }
        });
    }

    SessionDebouncer::~SessionDebouncer() {
        shutdown = true;
        cv.notify_all();
        if (worker.joinable()) {
            worker.join();
        }
    }

    // Call this instead of Session::initiate()
    void SessionDebouncer::trigger() {
        std::lock_guard<std::mutex> lock(mtx);
        // Set the trigger time
        triggerTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
        pending = true;
        cv.notify_one();
    }
}
