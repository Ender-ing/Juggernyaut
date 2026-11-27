/**
 * @brief
 * Non-blocking background debouncer for JugDriver
 * (NOTE: I ain't got no idea how this works exactly, Gemini did it TT)
**/

#include "DriverDebouncer.hpp"

namespace Driver {
    DriverDebouncer::DriverDebouncer(JugDriver& driver)
        : driverPtr(&driver) {

        worker = std::thread([this]() {
            constexpr auto pollDelay = std::chrono::milliseconds(50);

            while (!shutdown.load(std::memory_order_acquire)) {
                std::unique_lock<std::mutex> lock(mtx);

                // 1. Sleep with 0% CPU until a trigger is pending or we shut down
                cv.wait(lock, [this] {
                    return pending || shutdown.load(std::memory_order_acquire);
                });
                if (shutdown.load(std::memory_order_acquire)) return;

                // 2. Wait until 200ms have passed since the LAST trigger() call
                auto now = std::chrono::steady_clock::now();
                do {
                    cv.wait_until(lock, triggerTime);
                    if (shutdown.load(std::memory_order_acquire)) return;

                    now = std::chrono::steady_clock::now();
                    if (!allowRuns.load(std::memory_order_acquire) && now >= triggerTime - pollDelay) {
                        triggerTime += pollDelay;
                    }
                } while (now < triggerTime);

                // 3. Quiet window elapsed! Clear pending flag and unlock BEFORE running
                pending = false;
                lock.unlock();

                // 4. Run JugDriver on this background thread (VFS enters Safe Mode inside run())
                driverPtr->run();
            }
        });
    }

    DriverDebouncer::~DriverDebouncer() {
        shutdown.store(true, std::memory_order_release);
        cv.notify_all();
        if (worker.joinable()) {
            worker.join();
        }
    }

    void DriverDebouncer::trigger() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            triggerTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
            pending = true;
        }
        cv.notify_one();
    }
}
