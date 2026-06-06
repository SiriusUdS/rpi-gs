#pragma once

#include "server.h"
#include "UdpClient.h"
#include "Engine.h"
#include "FillStation.h"
#include "ThreadSafeQueue.h"
#include <vector>
#include <string>
#include <mutex>
#include <cstdint>
#include <thread>
#include <atomic>
#include <chrono>

#define UNSAFE_STATE_IDLE 1
#define UNSAFE_STATE_FIRE 2
#define UNSAFE_STATE_VALVE 3
#define UNSAFE_STATE_FILL 4


// Amount of control buttons
static constexpr int GS_CONTROL_BUTTON_AMOUNT = 8;

struct ButtonState {
    int bcm_pin;
    std::atomic<bool> is_pressed;
    int last_raw_state;
    int debounce_counter;
    uint64_t last_check_ms;
};

class GpioReader;

class GroundStation {
public:
    GroundStation(GpioReader& gpio_reader);
    ~GroundStation();

    // Prevent copy
    GroundStation(const GroundStation&) = delete;
    GroundStation& operator=(const GroundStation&) = delete;

    // Getters for core UDP components and devices
    UdpServer& getServer() { return server_; }
    UdpClient& getClient() { return client_; }
    Engine& getEngine() { return engine_; }
    FillStation& getFillStation() { return fill_; }

    // Getter for button states (thread-safe due to atomics)
    const ButtonState* getButtons() const { return buttons_; }

    // Error status control (thread-safe)
    bool getErrorFlagged() const { return error_flagged_.load(); }
    void setErrorFlagged(bool error);
    void toggleErrorFlag();

    bool canSendValve = false;

    bool valveActivate = false;

    bool igniterActivate = false;
    uint8_t unsafeState = UNSAFE_STATE_IDLE;

    // System Request State control (thread-safe)
    uint8_t getSystemRequestState() const { return system_request_state_.load(); }
    void setSystemRequestState(uint8_t state);

    // Logging interface (thread-safe)
    void log(const std::string& message);
    std::vector<std::string> getLogs(); // Returns a copy for thread safety
    void clearLogs();

    // Device connection / heartbeat status (thread-safe)
    bool isEngineConnected() const { return last_engine_udp_ticks_.load() <= 50; }
    bool isFillConnected() const { return last_fill_udp_ticks_.load() <= 50; }
    bool isServerDashboardConnected() const { return last_server_udp_ticks_.load() <= 50; }
    void resetEngineTimeout() { last_engine_udp_ticks_.store(0); }
    void resetFillTimeout() { last_fill_udp_ticks_.store(0); }
    void resetServerDashboardTimeout() { last_server_udp_ticks_.store(0); }

    // Send commands manually (e.g. overrides from Devices screen)
    void sendDeviceStatePacket(uint8_t device_id, uint8_t payload_id, uint32_t state_val);

    // Redraw flag check for TUI sync
    bool checkAndClearRedraw() {
        return redraw_flag_.exchange(false);
    }
    void triggerRedraw() {
        redraw_flag_.store(true);
    }

    // Queuing interface for external manual message sends
    void enqueueClientSend(const std::vector<uint8_t>& data);
    void enqueueServerSend(const std::vector<uint8_t>& data);

    // Backward compatible tick method
    bool tick();

    // RX/TX Telemetry Getters
    uint32_t getServerRxPackets() const { return server_rx_packets_.load(); }
    uint32_t getServerTxPackets() const { return server_tx_packets_.load(); }
    uint64_t getServerRxBytes() const { return server_rx_bytes_.load(); }
    uint64_t getServerTxBytes() const { return server_tx_bytes_.load(); }

    uint32_t getClientRxPackets() const { return client_rx_packets_.load(); }
    uint32_t getClientTxPackets() const { return client_tx_packets_.load(); }
    uint64_t getClientRxBytes() const { return client_rx_bytes_.load(); }
    uint64_t getClientTxBytes() const { return client_tx_bytes_.load(); }
    uint32_t getServerCrcErrors() const { return server_crc_errors_.load(); }
    uint32_t getClientCrcErrors() const { return client_crc_errors_.load(); }
    bool validateCrc(const std::vector<uint8_t>& data) const;

    // FSM State Action Handlers
    void handle_state_init();
    void handle_state_safe();
    void handle_state_unsafe();
    void handle_state_abort();

private:
    GpioReader& gpio_reader_;
    UdpServer server_;
    UdpClient client_;
    Engine engine_;
    FillStation fill_;

    std::atomic<bool> error_flagged_;
    std::atomic<uint8_t> system_request_state_;
    
    std::vector<std::string> logs_;
    mutable std::mutex logs_mutex_;

    std::atomic<int> last_engine_udp_ticks_;
    std::atomic<int> last_fill_udp_ticks_;
    std::atomic<int> last_server_udp_ticks_;
    std::atomic<bool> redraw_flag_;

    // Queues for client <-> server asynchronous redirect
    ThreadSafeQueue<std::vector<uint8_t>> client_incoming_queue_;
    ThreadSafeQueue<std::vector<uint8_t>> server_incoming_queue_;
    ThreadSafeQueue<std::vector<uint8_t>> server_outgoing_queue_;
    ThreadSafeQueue<std::vector<uint8_t>> client_outgoing_queue_;

    // Button states
    ButtonState buttons_[GS_CONTROL_BUTTON_AMOUNT];

    // Telemetry counters
    std::atomic<uint32_t> server_rx_packets_;
    std::atomic<uint32_t> server_tx_packets_;
    std::atomic<uint64_t> server_rx_bytes_;
    std::atomic<uint64_t> server_tx_bytes_;

    std::atomic<uint32_t> client_rx_packets_;
    std::atomic<uint32_t> client_tx_packets_;
    std::atomic<uint64_t> client_rx_bytes_;
    std::atomic<uint64_t> client_tx_bytes_;
    std::atomic<uint32_t> server_crc_errors_;
    std::atomic<uint32_t> client_crc_errors_;

    // Background Threading
    std::thread worker_thread_;
    std::atomic<bool> running_;

    void run(); // Main worker thread loop method
    void processReceiving();
    void processStateMachine(std::chrono::steady_clock::time_point& last_timer);
    void processSending();

    void updateButtons();
    void onButtonStateChanged(int index, bool pressed);
    void sendGSStatusPacket();
    void updateFSM();

    void unsafeIdle();
    void unsafeFire();
    void unsafeValve();
    void unsafeFill();
};
