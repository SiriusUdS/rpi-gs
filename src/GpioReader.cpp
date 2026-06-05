#include "GpioReader.h"
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <cerrno>

GpioReader::GpioReader() : running(false), setup_failed(false), poll_interval_ms(50) {
}

GpioReader::~GpioReader() {
    stop();
}

void GpioReader::addPin(int pin) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Avoid duplicates
    for (int p : monitored_pins) {
        if (p == pin) return;
    }
    monitored_pins.push_back(pin);
    pin_states[pin] = 0; // Initial state: 0
}

int GpioReader::getPinValue(int pin) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (setup_failed.load()) {
        return -2; // setup failure
    }
    auto it = pin_states.find(pin);
    if (it != pin_states.end()) {
        return it->second;
    }
    return -1; // Unregistered
}

void GpioReader::start(int interval_ms) {
    if (running) return;

    poll_interval_ms = interval_ms;
    running = true;

    // Initialize libgpiod exactly once across instances/threads
    static std::once_flag setup_flag;
    std::call_once(setup_flag, [this]() {
        try {
            gpiod::chip chip("/dev/gpiochip0");
            auto builder = chip.prepare_request();
            builder.set_consumer("sirius-gs");

            gpiod::line_settings settings;
            settings.set_direction(gpiod::line::direction::INPUT)
                    .set_bias(gpiod::line::bias::PULL_DOWN);

            for (int pin : monitored_pins) {
                builder.add_line_settings(pin, settings);
            }

            request_ = std::make_unique<gpiod::line_request>(builder.do_request());
        } catch (const std::exception& e) {
            setup_failed.store(true);
            std::cerr << "GpioReader: Failed to initialize libgpiod: " << e.what() << std::endl;
        }
    });

    std::lock_guard<std::mutex> lock(mutex_);
    for (int pin : monitored_pins) {
        if (!setup_failed.load()) {
            pin_states[pin] = 0; // Initial state
        } else {
            pin_states[pin] = -2;
        }
    }

    if (!setup_failed.load()) {
        poll_thread = std::thread(&GpioReader::run, this);
    }
}

void GpioReader::stop() {
    if (!running) return;

    running = false;
    if (poll_thread.joinable()) {
        poll_thread.join();
    }
}

void GpioReader::run() {
    while (running) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!setup_failed.load() && request_) {
                try {
                    for (int pin : monitored_pins) {
                        bool val = (request_->get_value(pin) == gpiod::line::value::ACTIVE);
                        pin_states[pin] = val ? 1 : 0;
                    }
                } catch (const std::exception& e) {
                    setup_failed.store(true);
                    std::cerr << "GpioReader: Poll failed: " << e.what() << std::endl;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    }
}
