#include "GroundStation.h"
#include "config.h"
#include "system.h"
#include "Crc32.h"
#include <cerrno>
#include <fstream>
#include "sirius-headers-common/Engine/EngineState.h"
#include "sirius-headers-common/FillingStation/FillingStationState.h"
#include "sirius-headers-common/Ethernet/UDPFrame.h"
#include "sirius-headers-common/Telecommunication/BoardCommandV2.h"
#include "sirius-headers-common/Telecommunication/PacketHeaderVariable.h"
#include "sirius-headers-common/GSControl/GSControlState.h"
#include "sirius-headers-common/GSControl/GSControlStatus.h"
#include <cstring>
#include <iostream>
#include <chrono>

/**
 * VAVLE OK
 * 
 * UNSAFE OK
 * 
 * EMERGENCY FORCE THE ABORT STATE NO MATTER WHAT BUTTON WE CLICK ON IT
 * 
 * make the button without delay (like ish 100ms)
 */

#define ALLOW_FILL_BTN 0
#define ARM_VALVE_BTN 1
#define ARM_IGNITER_BTN 2
#define ALLOW_DUMP_BTN 3
#define EMERGENCY_STOP_BTN 4
#define FIRE_IGNITER_BTN 5
#define VALVE_START_BTN 6
#define UNSAFE_KEY_BTN 7

static const int button_pins[GS_CONTROL_BUTTON_AMOUNT] = {
    21, // ALLOW_FILL
    20, // ARM_VALVE
    19, // ARM_IGNITER
    26, // ALLOW_DUMP
    12, // EMERGENCY_STOP
    13, // FIRE_IGNITER
    16, // VALVE_START
    6  // UNSAFE_KEY
};

static const char* button_names[GS_CONTROL_BUTTON_AMOUNT] = {
    "ALLOW_FILL",
    "ARM_VALVE",
    "ARM_IGNITER",
    "ALLOW_DUMP",
    "EMERGENCY_STOP",
    "FIRE_IGNITER",
    "VALVE_START",
    "UNSAFE_KEY"
};

GroundStation::GroundStation(GpioReader& gpio_reader)
    : gpio_reader_(gpio_reader),
      client_(IP_ADDRESS, REMOTE_SERVER_PORT, CLIENT_PORT),
      error_flagged_(false),
      system_request_state_(GS_CONTROL_STATE_INIT),
      last_engine_udp_ticks_(100),
      last_fill_udp_ticks_(100),
      last_server_udp_ticks_(100),
      redraw_flag_(false),
      server_rx_packets_(0),
      server_tx_packets_(0),
      server_rx_bytes_(0),
      server_tx_bytes_(0),
      client_rx_packets_(0),
      client_tx_packets_(0),
      client_rx_bytes_(0),
      client_tx_bytes_(0),
      server_crc_errors_(0),
      client_crc_errors_(0),
      running_(true) {
    log("GS: Ground Station backend initialized.");

    // Verify GPIO initialization mode
#ifdef USING_LIBGPIOD
    log("GS: INFO - Initializing libgpiod v2 (BCM pin numbering) and internal pull-downs.");
#else
    log("GS: INFO - libgpiod v2 not enabled. Running in local simulation mode.");
#endif

    // Initialize button states and register pins in GpioReader
    for (int i = 0; i < GS_CONTROL_BUTTON_AMOUNT; i++) {
        buttons_[i].bcm_pin = button_pins[i];
        buttons_[i].is_pressed.store(false);
        buttons_[i].last_raw_state = -1; // Unknown initially
        buttons_[i].debounce_counter = 0;
        buttons_[i].last_check_ms = 0;

        gpio_reader_.addPin(button_pins[i]);
    }

    worker_thread_ = std::thread(&GroundStation::run, this);
}

GroundStation::~GroundStation() {
    running_ = false;
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void GroundStation::setErrorFlagged(bool error) {
    if (error_flagged_.load() != error) {
        error_flagged_.store(error);
        log(std::string("GS: Error flag set to ") + (error ? "TRUE" : "FALSE"));
        
        // Broadcast error status to remote server via queue
        sendDeviceStatePacket(GS_CONTROL_BOARD_ID, DIAGNOSE_PACKET_CODE, error ? 1 : 0);
        triggerRedraw();
    }
}

void GroundStation::toggleErrorFlag() {
    setErrorFlagged(!error_flagged_.load());
}

void GroundStation::setSystemRequestState(uint8_t state) {
    if (buttons_[EMERGENCY_STOP_BTN].is_pressed.load() && state != GS_CONTROL_STATE_ABORT) {
        return;
    }
    if (system_request_state_.load() != state) {
        system_request_state_.store(state);
        std::string state_name = "UNKNOWN";
        if (state == GS_CONTROL_STATE_INIT) state_name = "INIT";
        else if (state == GS_CONTROL_STATE_SAFE) state_name = "SAFE";
        else if (state == GS_CONTROL_STATE_UNSAFE) state_name = "UNSAFE";
        else if (state == GS_CONTROL_STATE_ABORT) state_name = "ABORT";
        log("GS: System Request State changed to " + state_name + " (0x" + std::to_string(state) + ")");
        
        // Broadcast new request state to devices via queue
        sendDeviceStatePacket(GS_CONTROL_BOARD_ID, REQUEST_STATE, state);
        triggerRedraw();
    }
}

void GroundStation::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    logs_.push_back(message);
    if (logs_.size() > 500) {
        logs_.erase(logs_.begin());
    }
    triggerRedraw();
}

std::vector<std::string> GroundStation::getLogs() {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    return logs_;
}

void GroundStation::clearLogs() {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    logs_.clear();
    logs_.push_back("GS: Logs cleared.");
    triggerRedraw();
}

void GroundStation::sendDeviceStatePacket(uint8_t device_id, uint8_t payload_id, uint32_t state_val) {
    UDPPacketHeader header;
    std::memset(&header, 0, sizeof(header));
    header.frame.deviceID = device_id;
    header.frame.payloadID = payload_id;
    header.frame.payloadLenght = sizeof(uint32_t);
    header.frame.deviceState = (uint8_t)state_val;
    header.frame.deviceTS_MS = 0;

    std::vector<uint8_t> packet(sizeof(FrameUDPPacketHeader) + sizeof(uint32_t) + sizeof(uint32_t));
    std::memcpy(packet.data(), header.bytes, sizeof(FrameUDPPacketHeader));
    std::memcpy(packet.data() + sizeof(FrameUDPPacketHeader), &state_val, sizeof(uint32_t));

    uint32_t crc = Crc32::calculate(packet.data() + sizeof(FrameUDPPacketHeader), sizeof(uint32_t));
    std::memcpy(packet.data() + sizeof(FrameUDPPacketHeader) + sizeof(uint32_t), &crc, sizeof(uint32_t));

    enqueueServerSend(packet);
}

void GroundStation::enqueueClientSend(const std::vector<uint8_t>& data) {
    client_outgoing_queue_.push(data);
}

void GroundStation::enqueueServerSend(const std::vector<uint8_t>& data) {
    server_outgoing_queue_.push(data);
}

bool GroundStation::tick() {
    return checkAndClearRedraw();
}

void GroundStation::updateButtons() {
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();

    for (int i = 0; i < GS_CONTROL_BUTTON_AMOUNT; i++) {
        auto& btn = buttons_[i];
        
        // Debounce read frequency: 4ms
        if (now_ms - btn.last_check_ms < 4) {
            continue;
        }
        btn.last_check_ms = now_ms;

        // Get pin value from GpioReader
        int raw_val = gpio_reader_.getPinValue(btn.bcm_pin);
        static bool logged_error[GS_CONTROL_BUTTON_AMOUNT] = {false};

        if (raw_val < 0) {
            if (!logged_error[i]) {
                std::string err_desc;
                if (raw_val == -1) {
                    err_desc = "pin not registered";
                } else if (raw_val == -2) {
                    err_desc = "libgpiod initialization failed";
                } else {
                    err_desc = "unknown GPIO error (" + std::to_string(raw_val) + ")";
                }
                log("GS: ERROR - BCM " + std::to_string(btn.bcm_pin) + " " + err_desc);
                logged_error[i] = true;
            }
            continue;
        } else {
            logged_error[i] = false;
        }

        // Direct state change (zero delay)
        bool new_pressed = (raw_val == 1);
        if (btn.is_pressed.load() != new_pressed) {
            btn.is_pressed.store(new_pressed);
            onButtonStateChanged(i, new_pressed);
        }
        btn.last_raw_state = raw_val;
    }
}

void GroundStation::onButtonStateChanged(int index, bool pressed) {
    std::string btn_name = button_names[index];
    std::string state_str = pressed ? "PRESSED" : "RELEASED";
    log("GS: Button " + btn_name + " state changed to " + state_str);

    // Transitions based on physical button states
    if (pressed) {
        if (index == 4) { // EMERGENCY_STOP
            setSystemRequestState(GS_CONTROL_STATE_ABORT);
        } else if (index == 7) { // UNSAFE_KEY
            setSystemRequestState(GS_CONTROL_STATE_UNSAFE);
        }
    } else {
        if (index == 7) { // UNSAFE_KEY released
            setSystemRequestState(GS_CONTROL_STATE_SAFE);
        }
    }

    // Broadcast updated GS status (all button bits + state)
    sendGSStatusPacket();
    triggerRedraw();
}

void GroundStation::sendGSStatusPacket() {
    GSControlStatus status;
    std::memset(&status, 0, sizeof(status));
    status.bits.state = getSystemRequestState();
    status.bits.isAllowFillSwitchOn = buttons_[0].is_pressed.load() ? 1 : 0;
    status.bits.isArmServoSwitchOn = buttons_[1].is_pressed.load() ? 1 : 0;
    status.bits.isArmIgniterSwitchOn = buttons_[2].is_pressed.load() ? 1 : 0;
    status.bits.isAllowDumpSwitchOn = buttons_[3].is_pressed.load() ? 1 : 0;
    status.bits.isEmergencyStopButtonPressed = buttons_[4].is_pressed.load() ? 1 : 0;
    status.bits.isFireIgniterButtonPressed = buttons_[5].is_pressed.load() ? 1 : 0;
    status.bits.isValveStartButtonPressed = buttons_[6].is_pressed.load() ? 1 : 0;
    status.bits.isUnsafeKeySwitchPressed = buttons_[7].is_pressed.load() ? 1 : 0;

    UDPPacketHeader header;
    std::memset(&header, 0, sizeof(header));
    header.frame.deviceID = GS_CONTROL_BOARD_ID;
    header.frame.payloadID = GET_SYSTEM; // telemetery status packet
    header.frame.payloadLenght = sizeof(uint16_t);
    header.frame.deviceState = getSystemRequestState();

    std::vector<uint8_t> packet(sizeof(FrameUDPPacketHeader) + sizeof(uint16_t) + sizeof(uint32_t));
    std::memcpy(packet.data(), header.bytes, sizeof(FrameUDPPacketHeader));
    std::memcpy(packet.data() + sizeof(FrameUDPPacketHeader), &status.value, sizeof(uint16_t));

    uint32_t crc = Crc32::calculate(packet.data() + sizeof(FrameUDPPacketHeader), sizeof(uint16_t));
    std::memcpy(packet.data() + sizeof(FrameUDPPacketHeader) + sizeof(uint16_t), &crc, sizeof(uint32_t));

    enqueueServerSend(packet);
}

void GroundStation::updateFSM() {
    uint8_t current_state = system_request_state_.load();
    canSendValve = false;
    switch (current_state) {
        case GS_CONTROL_STATE_INIT:
            handle_state_init();
            break;
        case GS_CONTROL_STATE_SAFE:
            handle_state_safe();
            break;
        case GS_CONTROL_STATE_UNSAFE:
            handle_state_unsafe();
            break;
        case GS_CONTROL_STATE_ABORT:
            handle_state_abort();
            break;
        default:
            break;
    }
}

void GroundStation::unsafeIdle()
{
    if(buttons_[ALLOW_FILL_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_FILL;
    }else if(buttons_[FIRE_IGNITER_BTN].is_pressed && buttons_[ARM_IGNITER_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_FIRE;
    }
}

void GroundStation::unsafeFire()
{
    if(!buttons_[ARM_IGNITER_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_IDLE;
    }else if(buttons_[ARM_VALVE_BTN].is_pressed && buttons_[VALVE_START_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_VALVE;
    }
}

void GroundStation::unsafeValve()
{
    if(!buttons_[ARM_VALVE_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_IDLE;
    }
}

void GroundStation::unsafeFill()
{
    if(!buttons_[ALLOW_FILL_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_IDLE;
    }
    canSendValve = true;
}

void GroundStation::handle_state_init() {
    // Stubs for actions to do at INIT state

    unsafeState = UNSAFE_STATE_IDLE;
}

void GroundStation::handle_state_safe() {
    // Stubs for actions to do at SAFE state
    valveActivate = false;
    igniterActivate = false;
    unsafeState = UNSAFE_STATE_IDLE;
}

void GroundStation::handle_state_unsafe() {
    // Stubs for actions to do at UNSAFE state
    switch(unsafeState){
        case UNSAFE_STATE_IDLE:
            unsafeIdle();
            break;
        case UNSAFE_STATE_FILL:
            unsafeFill();
            break;
        case UNSAFE_STATE_FIRE:
            unsafeFire();
            break;
        case UNSAFE_STATE_VALVE:
            unsafeValve();
            break;
        
    }
}

void GroundStation::handle_state_abort() {
    // Stubs for actions to do at ABORT state
    unsafeState = UNSAFE_STATE_IDLE;
}

void GroundStation::processReceiving() {
    UdpMessage server_msg;
    while (server_.receive(server_msg)) {
        if (!validateCrc(server_msg.data)) {
            log("GS: CRC32 mismatch on local server link (from client device)! Discarding packet.");
            server_crc_errors_++;
            continue;
        }
        server_rx_packets_++;
        server_rx_bytes_ += server_msg.data.size();
        client_incoming_queue_.push(server_msg.data);
    }

    UdpClientMessage client_msg;
    while (client_.receive(client_msg)) {
        if (!validateCrc(client_msg.data)) {
            log("GS: CRC32 mismatch on local client link (from remote server)! Discarding packet.");
            client_crc_errors_++;
            continue;
        }
        client_rx_packets_++;
        client_rx_bytes_ += client_msg.data.size();
        server_incoming_queue_.push(client_msg.data);
        last_server_udp_ticks_.store(0);
    }
}

void GroundStation::processStateMachine(std::chrono::steady_clock::time_point& last_timer) {
    updateButtons();

    updateFSM();

    // Increment device connection timeouts every 100ms
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_timer).count();
    if (elapsed >= 100) {
        last_engine_udp_ticks_++;
        last_fill_udp_ticks_++;
        last_server_udp_ticks_++;
        last_timer = now;
        triggerRedraw();
    }

    std::vector<uint8_t> client_data;
    while (client_incoming_queue_.pop(client_data)) {
        server_outgoing_queue_.push(client_data);

        if (client_data.size() >= sizeof(FrameUDPPacketHeader)) {
            UDPPacketHeader packet_header;
            std::memcpy(packet_header.bytes, client_data.data(), sizeof(FrameUDPPacketHeader));

            uint32_t state_val = 0;
            if (client_data.size() >= sizeof(FrameUDPPacketHeader) + sizeof(uint32_t)) {
                std::memcpy(&state_val, client_data.data() + sizeof(FrameUDPPacketHeader), sizeof(uint32_t));
            } else {
                state_val = packet_header.frame.deviceState;
            }

            if (packet_header.frame.payloadID == SYNC_PACKET_CODE || 
                packet_header.frame.payloadID == REQUEST_STATE) {
                
                if (packet_header.frame.deviceID == ENGINE_BOARD_ID) {
                    engine_.setState((uint8_t)state_val);
                    resetEngineTimeout();
                    log("GS: RX Engine Board State Sync: 0x" + std::to_string(state_val));
                } else if (packet_header.frame.deviceID == FILLING_STATION_BOARD_ID) {
                    fill_.setState((uint8_t)state_val);
                    resetFillTimeout();
                    log("GS: RX Fill Station Board State Sync: 0x" + std::to_string(state_val));
                }
            }
            log("GS: Redirected client packet (Device: " + std::to_string(packet_header.frame.deviceID) + ") to Server queue");
            triggerRedraw();
        }
    }

    std::vector<uint8_t> server_data;
    while (server_incoming_queue_.pop(server_data)) {
        if(canSendValve){
            client_outgoing_queue_.push(server_data);
        }
    }
}

void GroundStation::processSending() {
    std::vector<uint8_t> out_server_data;
    while (server_outgoing_queue_.pop(out_server_data)) {
        if (client_.send(out_server_data)) {
            client_tx_packets_++;
            client_tx_bytes_ += out_server_data.size();
        }
    }

    std::vector<uint8_t> out_client_data;
    while (client_outgoing_queue_.pop(out_client_data)) {
        if (server_.send(out_client_data)) {
            server_tx_packets_++;
            server_tx_bytes_ += out_client_data.size();
        }
    }
}

void GroundStation::run() {
    auto last_timer = std::chrono::steady_clock::now();
    
    while (running_) {
        processReceiving();
        processStateMachine(last_timer);
        processSending();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

bool GroundStation::validateCrc(const std::vector<uint8_t>& data) const {
    size_t N = data.size();
    if (N < sizeof(FrameUDPPacketHeader) + sizeof(uint32_t)) {
        return false;
    }
    
    // Calculate CRC of the payload only (excluding the 12-byte header)
    uint32_t calculated_crc = Crc32::calculate(data.data() + sizeof(FrameUDPPacketHeader), N - sizeof(FrameUDPPacketHeader) - sizeof(uint32_t));
    
    // Extract the received CRC from the last 4 bytes
    uint32_t received_crc = 0;
    std::memcpy(&received_crc, &data[N - 4], sizeof(uint32_t));
    
    return calculated_crc == received_crc;
}
