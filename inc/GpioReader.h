#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

#ifdef USING_LIBGPIOD
#include <gpiod.hpp>
#else
// Mock/Stub implementation of libgpiod v2 for local PC/WSL simulator builds
namespace gpiod {
    namespace line {
        enum class direction {
            INPUT,
            OUTPUT
        };
        enum class bias {
            DISABLED,
            PULL_UP,
            PULL_DOWN,
            AS_IS
        };
        enum class value {
            INACTIVE = 0,
            ACTIVE = 1
        };
    }

    class line_settings {
    public:
        line_settings() {}
        line_settings& set_direction(line::direction dir) { (void)dir; return *this; }
        line_settings& set_bias(line::bias b) { (void)b; return *this; }
    };

    class line_request {
    public:
        line_request() {}
        line::value get_value(unsigned int offset) { (void)offset; return line::value::INACTIVE; }
    };

    class request_builder {
    public:
        request_builder() {}
        request_builder& set_consumer(const std::string& consumer) { (void)consumer; return *this; }
        request_builder& add_line_settings(unsigned int offset, const line_settings& settings) {
            (void)offset; (void)settings;
            return *this;
        }
        line_request do_request() { return line_request(); }
    };

    class chip {
    public:
        chip(const std::string& path) { (void)path; }
        request_builder prepare_request() { return request_builder(); }
    };
}
#endif

class GpioReader {
public:
    GpioReader();
    ~GpioReader();

    // Prevent copying
    GpioReader(const GpioReader&) = delete;
    GpioReader& operator=(const GpioReader&) = delete;

    /**
     * @brief Register a GPIO pin to be monitored.
     * @param pin The GPIO pin number (BCM numbering).
     */
    void addPin(int pin);

    /**
     * @brief Retrieve the last read value of a monitored pin.
     * @param pin The BCM GPIO pin number.
     * @return 0 for low, 1 for high, -1 if pin is not monitored or error.
     */
    int getPinValue(int pin);

    /**
     * @brief Starts the background polling thread.
     * @param interval_ms The polling interval in milliseconds.
     */
    void start(int interval_ms = 50);

    /**
     * @brief Stops the background thread.
     */
    void stop();

    // Set callback for routing logs
    void setLogCallback(std::function<void(const std::string&)> cb) { log_callback_ = cb; }

private:
    void run();

    std::vector<int> monitored_pins;
    std::unordered_map<int, int> pin_states;
    std::mutex mutex_;
    std::thread poll_thread;
    std::atomic<bool> running;
    std::atomic<bool> setup_failed;
    int poll_interval_ms;
    std::unique_ptr<gpiod::line_request> request_;
    std::function<void(const std::string&)> log_callback_;
};
