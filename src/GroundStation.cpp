#include "GroundStation.h"
#include "config.h"
#include "system.h"
#include "Crc32.h"
#include <cerrno>
#include <fstream>
#include "system/state.hpp"
#include "system/board_id.hpp"
#include "system/crc32_polynomial.hpp"
#include "framing/ethernet_header.hpp"
#include "framing/payload_type.hpp"
#include "response/response_type.hpp"
#include "command/command_type.hpp"
#include "telemetry/telemetry_type.hpp"
#include "telemetry/fcu_system_state.hpp"
#include "telemetry/ecu_system_state.hpp"
#include "telemetry/gs_system_state.hpp"
#include "command/command_type.hpp"
#include "sirius-headers-common/GSControl/GSControlStatus.h"
#include "system/valves/ecu.hpp"
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
#define VALVE_START_BTN 5
#define FIRE_IGNITER_BTN 6
#define UNSAFE_KEY_BTN 7

static const int button_pins[GS_CONTROL_BUTTON_AMOUNT] = {
    21, // ALLOW_FILL
    20, // ARM_VALVE
    19, // ARM_IGNITER
    26, // ALLOW_DUMP
    EMERGENCY_STOP_PIN, // EMERGENCY_STOP
    16, // VALVE_START
    13, // FIRE_IGNITER
    6  // UNSAFE_KEY
};

static const char* button_names[GS_CONTROL_BUTTON_AMOUNT] = {
    "ALLOW_FILL",
    "ARM_VALVE",
    "ARM_IGNITER",
    "ALLOW_DUMP",
    "EMERGENCY_STOP",
    "FIRE_IGNITER",
    "VALVE START",
    "UNSAFE_KEY"
};

GroundStation::GroundStation(GpioReader& gpio_reader)
    : gpio_reader_(gpio_reader),
      client_(IP_ADDRESS, REMOTE_SERVER_PORT, CLIENT_PORT),
      error_flagged_(false),
      system_request_state_(static_cast<uint8_t>(logic::control::State::Init)),
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
      device_commands_sent_(0),
      device_acks_received_(0),
      running_(true),
      seq_counter_(1) {
    server_.setLogCallback([this](const std::string& msg) { log(msg); });
    client_.setLogCallback([this](const std::string& msg) { log(msg); });
    gpio_reader_.setLogCallback([this](const std::string& msg) { log(msg); });

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

    // Register menu navigation buttons
    gpio_reader_.addPin(22); // MENU_UP
    gpio_reader_.addPin(23); // MENU_DOWN
    gpio_reader_.addPin(24); // MENU_SELECT

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
        sendDeviceStatePacket(static_cast<uint8_t>(BoardId::GsControl), 0x02, error ? 1 : 0);
        triggerRedraw();
    }
}

void GroundStation::toggleErrorFlag() {
    setErrorFlagged(!error_flagged_.load());
}

void GroundStation::setSystemRequestState(uint8_t state) {
    if (buttons_[EMERGENCY_STOP_BTN].is_pressed.load() && state != static_cast<uint8_t>(logic::control::State::Abort)) {
        return;
    }
    uint8_t old_state = system_request_state_.load();
    if (old_state != state) {
        system_request_state_.store(state);
        
        std::string old_state_name = "UNKNOWN";
        if (old_state == static_cast<uint8_t>(logic::control::State::Init)) old_state_name = "INIT";
        else if (old_state == static_cast<uint8_t>(logic::control::State::Safe)) old_state_name = "SAFE";
        else if (old_state == static_cast<uint8_t>(logic::control::State::Unsafe)) old_state_name = "UNSAFE";
        else if (old_state == static_cast<uint8_t>(logic::control::State::Abort)) old_state_name = "ABORT";

        std::string state_name = "UNKNOWN";
        if (state == static_cast<uint8_t>(logic::control::State::Init)) state_name = "INIT";
        else if (state == static_cast<uint8_t>(logic::control::State::Safe)) state_name = "SAFE";
        else if (state == static_cast<uint8_t>(logic::control::State::Unsafe)) state_name = "UNSAFE";
        else if (state == static_cast<uint8_t>(logic::control::State::Abort)) state_name = "ABORT";
        
        log("GS: State transition from " + old_state_name + " to " + state_name);
        
        // Broadcast new request state to devices via queue
        sendDeviceStatePacket(static_cast<uint8_t>(BoardId::GsControl), static_cast<uint8_t>(logic::communication::command::CommandType::SetState), state);
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
    uint8_t type = static_cast<uint8_t>(PayloadType::Command);
    uint8_t id = payload_id;
    uint8_t target = static_cast<uint8_t>(BoardId::Broadcast);

    // Map legacy payload_id if needed
    if (payload_id == 0x83) { // REQUEST_STATE legacy code
        id = static_cast<uint8_t>(logic::communication::command::CommandType::SetState);
    }

    std::vector<uint8_t> packet(sizeof(EthernetHeader) + 4 + sizeof(uint32_t));
    EthernetHeader header;
    std::memset(&header, 0, sizeof(header));
    header.sender_id = static_cast<uint32_t>(BoardId::GsControl);
    header.target_id = target;
    header.payload_type = type;
    header.payload_id = id;
    header.payload_size_bytes = 4; // padded to 4 bytes
    header.sender_state = getSystemRequestState();
    header.seq = getNextSeq();
    header.sender_timestamp_ms = 0;

    std::memcpy(packet.data(), &header, sizeof(EthernetHeader));

    // Payload: use SetStateFrame for SetState command, copy 4 bytes (padded)
    uint8_t payload[4] = {0};
    if (id == static_cast<uint8_t>(logic::communication::command::CommandType::SetState)) {
        SetStateFrame frame;
        frame.flags = 0;
        frame.requestedID = static_cast<uint8_t>(state_val);
        std::memcpy(payload, &frame, sizeof(SetStateFrame));
    } else {
        uint32_t val = state_val;
        std::memcpy(payload, &val, sizeof(uint32_t));
    }
    std::memcpy(packet.data() + sizeof(EthernetHeader), payload, 4);

    // CRC-32 computed over EthernetHeader + padded Payload
    uint32_t crc = Crc32::calculate(packet.data(), sizeof(EthernetHeader) + 4);
    std::memcpy(packet.data() + sizeof(EthernetHeader) + 4, &crc, sizeof(uint32_t));

    if (id == static_cast<uint8_t>(logic::communication::command::CommandType::SetState)) {
        pending_cmd_.active = true;
        pending_cmd_.seq = header.seq;
        pending_cmd_.state_val = static_cast<uint8_t>(state_val);
        pending_cmd_.last_sent = std::chrono::steady_clock::now();
        pending_cmd_.acked = false;

        enqueueClientSend(packet);
    }
    enqueueServerSend(packet);
}

void GroundStation::enqueueClientSend(const std::vector<uint8_t>& data) {
    if (data.size() >= sizeof(EthernetHeader)) {
        EthernetHeader header;
        std::memcpy(&header, data.data(), sizeof(EthernetHeader));
        
        if (header.payload_type == static_cast<uint8_t>(PayloadType::Command)) {
            device_commands_sent_++;
        }
    }
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
        bool new_pressed = (i == EMERGENCY_STOP_BTN) ? (raw_val == 0) : (raw_val == 1);
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
            setSystemRequestState(static_cast<uint8_t>(logic::control::State::Abort));
        } else if (index == 7) { // UNSAFE_KEY
            setSystemRequestState(static_cast<uint8_t>(logic::control::State::Unsafe));
        }
    } else {
        if (index == 7) { // UNSAFE_KEY released
            setSystemRequestState(static_cast<uint8_t>(logic::control::State::Safe));
        }else if (index == 4){
            setSystemRequestState(static_cast<uint8_t>(logic::control::State::Safe));
        }
    }

    // Broadcast updated GS status (all button bits + state)
    sendGSStatusPacket();
    triggerRedraw();
}

void GroundStation::sendGSStatusPacket() {
    GSSystemState status;
    std::memset(&status, 0, sizeof(status));
    status.isAllowFillSwitchOn = buttons_[ALLOW_FILL_BTN].is_pressed.load() ? 1 : 0;
    status.isArmServoSwitchOn = buttons_[ARM_VALVE_BTN].is_pressed.load() ? 1 : 0;
    status.isArmIgniterSwitchOn = buttons_[ARM_IGNITER_BTN].is_pressed.load() ? 1 : 0;
    status.isAllowDumpSwitchOn = buttons_[ALLOW_DUMP_BTN].is_pressed.load() ? 1 : 0;
    status.isEmergencyStopButtonPressed = buttons_[EMERGENCY_STOP_BTN].is_pressed.load() ? 0 : 1;
    status.isFireIgniterButtonPressed = buttons_[FIRE_IGNITER_BTN].is_pressed.load() ? 1 : 0;
    status.isValveStartButtonPressed = buttons_[VALVE_START_BTN].is_pressed.load() ? 1 : 0;
    status.isUnsafeKeySwitchPressed = buttons_[UNSAFE_KEY_BTN].is_pressed.load() ? 1 : 0;
    status.reserved = 0;
    status.fcuState = fill_.getState();
    status.ecuState = engine_.getState();

    std::vector<uint8_t> packet(sizeof(EthernetHeader) + sizeof(GSSystemState) + sizeof(uint32_t));
    EthernetHeader header;
    std::memset(&header, 0, sizeof(header));
    header.sender_id = static_cast<uint32_t>(BoardId::GsControl);
    header.target_id = static_cast<uint32_t>(BoardId::Broadcast);
    header.payload_type = static_cast<uint32_t>(PayloadType::Telemetry);
    header.payload_id = static_cast<uint8_t>(TelemetryType::SystemState); // 0x01 fits in 6 bits
    header.payload_size_bytes = sizeof(GSSystemState);
    header.sender_state = engine_.getState();
    header.seq = 0;
    header.sender_timestamp_ms = 0;

    std::memcpy(packet.data(), &header, sizeof(EthernetHeader));
    std::memcpy(packet.data() + sizeof(EthernetHeader), &status, sizeof(GSSystemState));

    // CRC-32 computed over EthernetHeader + GSSystemState payload
    uint32_t crc = Crc32::calculate(packet.data(), sizeof(EthernetHeader) + sizeof(GSSystemState));
    std::memcpy(packet.data() + sizeof(EthernetHeader) + sizeof(GSSystemState), &crc, sizeof(uint32_t));

    enqueueServerSend(packet);
}

void GroundStation::updateFSM() {
    uint8_t current_state = system_request_state_.load();
    canSendValve = false;
    if (current_state == static_cast<uint8_t>(logic::control::State::Init)) {
        handle_state_init();
    } else if (current_state == static_cast<uint8_t>(logic::control::State::Safe)) {
        handle_state_safe();
    } else if (current_state == static_cast<uint8_t>(logic::control::State::Unsafe)) {
        handle_state_unsafe();
    } else if (current_state == static_cast<uint8_t>(logic::control::State::Abort)) {
        handle_state_abort();
    }
}

void GroundStation::unsafeIdle()
{
    if(buttons_[ALLOW_FILL_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_FILL;
    }else if(buttons_[VALVE_START_BTN].is_pressed && buttons_[ARM_IGNITER_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_FIRE;
    }
}

void GroundStation::unsafeFire()
{
    if(!buttons_[ARM_IGNITER_BTN].is_pressed){
        unsafeState = UNSAFE_STATE_IDLE;
    }else if(buttons_[ARM_VALVE_BTN].is_pressed && buttons_[FIRE_IGNITER_BTN].is_pressed){
        if (engine_.getState() == static_cast<uint8_t>(logic::control::State::Ignite) &&
            fill_.getState() == static_cast<uint8_t>(logic::control::State::Ignite)) {
            unsafeState = UNSAFE_STATE_VALVE;
        }
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
    uint8_t prev = unsafeState;
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
    if (unsafeState != prev) {
        onUnsafeStateChanged(prev, unsafeState);
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



    // Retry sending pending unsafe command every 100ms
    if (pending_cmd_.active && !pending_cmd_.acked) {
        auto now_steady = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now_steady - pending_cmd_.last_sent).count();
        if (elapsed_ms >= 100) {
            transmitUnsafeCommand(pending_cmd_.state_val, pending_cmd_.seq);
            pending_cmd_.last_sent = now_steady;
        }
    }

    // Increment device connection timeouts every 100ms
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_timer).count();
    if (elapsed >= 100) {
        last_engine_udp_ticks_++;
        last_fill_udp_ticks_++;
        last_server_udp_ticks_++;
        last_timer = now;
        sendTelemetryPacket();
        triggerRedraw();
    }

    std::vector<uint8_t> client_data;
    while (client_incoming_queue_.pop(client_data)) {
        server_outgoing_queue_.push(client_data);

        if (client_data.size() >= sizeof(EthernetHeader)) {
            EthernetHeader header;
            std::memcpy(&header, client_data.data(), sizeof(EthernetHeader));

            uint8_t state_val = header.sender_state;

            bool is_ack = (header.payload_type == static_cast<uint8_t>(PayloadType::Response)) &&
                          (header.payload_id == static_cast<uint8_t>(ResponseType::Ack));

            if (is_ack) {
                device_acks_received_++;
            }

            if (is_ack && pending_cmd_.active && header.seq == pending_cmd_.seq) {
                pending_cmd_.acked = true;
                std::string cmd_name = getStateName(pending_cmd_.state_val);
                log("GS: Command " + cmd_name + " [seq=" + std::to_string(header.seq) + "] ACK received from device " + std::to_string(header.sender_id));
            }

            if (header.sender_id == static_cast<uint8_t>(BoardId::Engine)) {
                uint8_t old_state = engine_.getState();
                if (old_state != state_val) {
                    engine_.setState(state_val);
                    log("GS: Engine Board state transitioned from " + getStateName(old_state) + " to " + getStateName(state_val));
                }
                resetEngineTimeout();
                if (is_ack) {
                    log("GS: RX Engine Board State Sync (Ack): 0x" + std::to_string(state_val));
                } else {
                    log("GS: RX Engine Board State Sync: 0x" + std::to_string(state_val));
                }
            } else if (header.sender_id == static_cast<uint8_t>(BoardId::FillingStation)) {
                uint8_t old_state = fill_.getState();
                if (old_state != state_val) {
                    fill_.setState(state_val);
                    log("GS: Fill Station Board state transitioned from " + getStateName(old_state) + " to " + getStateName(state_val));
                }
                resetFillTimeout();
                if (is_ack) {
                    log("GS: RX Fill Station Board State Sync (Ack): 0x" + std::to_string(state_val));
                } else {
                    log("GS: RX Fill Station Board State Sync: 0x" + std::to_string(state_val));
                }
            }
            log("GS: Redirected client packet (Device: " + std::to_string(header.sender_id) + ") to Server queue");
            triggerRedraw();
        }
    }

    std::vector<uint8_t> server_data;
    while (server_incoming_queue_.pop(server_data)) {
        if (server_data.size() >= sizeof(EthernetHeader)) {
            EthernetHeader header;
            std::memcpy(&header, server_data.data(), sizeof(EthernetHeader));

            // 1. Handle SystemState Telemetry from remote server
            if (header.payload_type == static_cast<uint8_t>(PayloadType::Telemetry) &&
                header.payload_id == static_cast<uint8_t>(TelemetryType::SystemState)) {
                if (server_data.size() >= sizeof(EthernetHeader) + sizeof(GSSystemState)) {
                    GSSystemState status;
                    std::memcpy(&status, server_data.data() + sizeof(EthernetHeader), sizeof(GSSystemState));

                    uint8_t old_fill_state = fill_.getState();
                    uint8_t old_engine_state = engine_.getState();

                    if (old_fill_state != status.fcuState) {
                        fill_.setState(status.fcuState);
                        log("GS: Fill Station Board state transitioned from " + getStateName(old_fill_state) + " to " + getStateName(status.fcuState));
                    }
                    if (old_engine_state != status.ecuState) {
                        engine_.setState(status.ecuState);
                        log("GS: Engine Board state transitioned from " + getStateName(old_engine_state) + " to " + getStateName(status.ecuState));
                    }

                    resetFillTimeout();
                    resetEngineTimeout();
                    resetServerDashboardTimeout();

                    log("GS: RX GS System State from Computer: FCU 0x" + std::to_string(status.fcuState) + ", ECU 0x" + std::to_string(status.ecuState));
                    triggerRedraw();
                }
            }else if(header.payload_type == static_cast<uint8_t>(PayloadType::Command) && header.payload_id == static_cast<uint8_t>(logic::communication::command::CommandType::SetValvePosition)){
                SetValvePositionFrame valv;
                std::memcpy(&valv, server_data.data()+ sizeof(EthernetHeader), sizeof(SetValvePositionFrame));
                if(unsafeState == UNSAFE_STATE_FILL && buttons_[ALLOW_DUMP_BTN].is_pressed.load()){
                    if(static_cast<logic::control::State>(fill_.getState()) == logic::control::State::Unsafe){
                        if(static_cast<EcuValves>(valv.valve) == EcuValves::NOS || static_cast<EcuValves>(valv.valve) == EcuValves::IPA){
                            enqueueClientSend(server_data);
                        }
                    }
                }else if(static_cast<logic::control::State>(fill_.getState()) == logic::control::State::Unsafe && unsafeState == UNSAFE_STATE_FILL){
                    if(valv.valve == FcuValves::Fill){
                        enqueueClientSend(server_data);
                    }else{
                        log("GS: unknown valve");
                    }
                    continue;
                }else if(static_cast<logic::control::State>(fill_.getState()) == logic::control::State::Unsafe && buttons_[ALLOW_DUMP_BTN].is_pressed.load() && valv.valve == FcuValves::Dump){
                    enqueueClientSend(server_data);
                    log("GS: Dump sent");
                }else{
                    log("GS: Fill station not in unsafe mode or allowing fill not allowed");
                }
                
            }else if (header.payload_type == static_cast<uint8_t>(PayloadType::Command) && (header.payload_id == static_cast<uint8_t>(logic::communication::command::CommandType::SetControlFlag) || header.payload_id == static_cast<uint8_t>(logic::communication::command::CommandType::Ping))){
                enqueueClientSend(server_data);
                log("GS: Sent Control flags");
            }
        }
    }
}

void GroundStation::processSending() {
    std::vector<uint8_t> out_server_data;
    while (server_outgoing_queue_.pop(out_server_data)) {
        if (out_server_data.size() >= sizeof(EthernetHeader) + sizeof(uint32_t)) {
            size_t N = out_server_data.size();
            uint32_t crc = Crc32::calculate(out_server_data.data(), N - sizeof(uint32_t));
            std::memcpy(out_server_data.data() + N - sizeof(uint32_t), &crc, sizeof(uint32_t));
        }
        if (client_.send(out_server_data)) {
            client_tx_packets_++;
            client_tx_bytes_ += out_server_data.size();
        }
    }

    std::vector<uint8_t> out_client_data;
    while (client_outgoing_queue_.pop(out_client_data)) {
        if (out_client_data.size() >= sizeof(EthernetHeader) + sizeof(uint32_t)) {
            size_t N = out_client_data.size();
            uint32_t crc = Crc32::calculate(out_client_data.data(), N - sizeof(uint32_t));
            std::memcpy(out_client_data.data() + N - sizeof(uint32_t), &crc, sizeof(uint32_t));
        }
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

void GroundStation::sendTelemetryPacket() {
    sendGSStatusPacket();
}


bool GroundStation::validateCrc(const std::vector<uint8_t>& data) const {
    size_t N = data.size();
    if (N < sizeof(EthernetHeader) + sizeof(uint32_t)) {
        return false;
    }
    
    // Calculate CRC of the header + payload (excluding the last 4 bytes of CRC)
    uint32_t calculated_crc = Crc32::calculate(data.data(), N - sizeof(uint32_t));
    
    // Extract the received CRC from the last 4 bytes
    uint32_t received_crc = 0;
    std::memcpy(&received_crc, &data[N - 4], sizeof(uint32_t));
    
    return calculated_crc == received_crc;
}

uint8_t GroundStation::getNextSeq() {
    uint8_t seq = seq_counter_.load();
    uint8_t next_seq = (seq % 15) + 1; // 1 to 15
    seq_counter_.store(next_seq);
    return seq;
}

std::string GroundStation::getStateName(uint8_t state) {
    switch (static_cast<logic::control::State>(state)) {
        case logic::control::State::Init: return "INIT";
        case logic::control::State::Safe: return "SAFE";
        case logic::control::State::Unsafe: return "UNSAFE";
        case logic::control::State::Abort: return "ABORT";
        case logic::control::State::Error: return "ERROR";
        case logic::control::State::Ignite: return "IGNITE";
        case logic::control::State::Launch: return "LAUNCH";
        case logic::control::State::Test: return "TEST";
        default: return "UNKNOWN (" + std::to_string(state) + ")";
    }
}

void GroundStation::sendUnsafeCommand(uint8_t requested_state) {
    uint8_t seq = getNextSeq();

    pending_cmd_.active = true;
    pending_cmd_.seq = seq;
    pending_cmd_.state_val = requested_state;
    pending_cmd_.last_sent = std::chrono::steady_clock::now();
    pending_cmd_.acked = false;

    transmitUnsafeCommand(requested_state, seq);
}

void GroundStation::transmitUnsafeCommand(uint8_t requested_state, uint8_t seq) {
    uint8_t type = static_cast<uint8_t>(PayloadType::Command);
    uint8_t id = static_cast<uint8_t>(logic::communication::command::CommandType::SetState);
    uint8_t target = static_cast<uint8_t>(BoardId::Broadcast);

    std::vector<uint8_t> packet(sizeof(EthernetHeader) + 4 + sizeof(uint32_t));
    EthernetHeader header;
    std::memset(&header, 0, sizeof(header));
    header.sender_id = static_cast<uint32_t>(BoardId::GsControl);
    header.target_id = target;
    header.payload_type = type;
    header.payload_id = id;
    header.payload_size_bytes = 4; // padded to 4 bytes
    header.sender_state = getSystemRequestState();
    header.seq = seq;
    header.sender_timestamp_ms = 0;

    std::memcpy(packet.data(), &header, sizeof(EthernetHeader));

    // Payload: SetStateFrame: Byte 0: flags, Byte 1: requestedID, padded to 4 bytes
    SetStateFrame frame;
    frame.flags = 0;
    frame.requestedID = requested_state;

    uint8_t payload[4] = {0};
    std::memcpy(payload, &frame, sizeof(SetStateFrame));
    std::memcpy(packet.data() + sizeof(EthernetHeader), payload, 4);

    // CRC-32 computed over EthernetHeader + padded Payload
    uint32_t crc = Crc32::calculate(packet.data(), sizeof(EthernetHeader) + 4);
    std::memcpy(packet.data() + sizeof(EthernetHeader) + 4, &crc, sizeof(uint32_t));

    // Enqueue for sending to devices
    enqueueClientSend(packet);

    std::string cmd_name = getStateName(requested_state);
    log("GS: Sent command " + cmd_name + " (0x" + std::to_string(requested_state) + ") to devices [seq=" + std::to_string(seq) + "]");
}

void GroundStation::clearPendingCommand() {
    if (pending_cmd_.active) {
        pending_cmd_.active = false;
        pending_cmd_.seq = 0;
        pending_cmd_.state_val = 0;
        pending_cmd_.acked = false;
        log("GS: Cleared pending command.");
    }
}

void GroundStation::onUnsafeStateChanged(uint8_t prev, uint8_t current) {
    (void)prev; // Unused parameter
    if (current == UNSAFE_STATE_FIRE) {
        sendUnsafeCommand(static_cast<uint8_t>(logic::control::State::Ignite));
    } else if (current == UNSAFE_STATE_VALVE) {
        sendUnsafeCommand(static_cast<uint8_t>(logic::control::State::Launch));
    } else {
        clearPendingCommand();
    }
}
