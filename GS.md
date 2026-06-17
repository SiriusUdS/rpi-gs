# GroundStation.cpp Methods Documentation

This document provides a comprehensive overview of all methods implemented in `GroundStation.cpp`. Each entry details the method's parameters, return values, and main functional purpose.

## Core Lifecycle & FSM

### `GroundStation(GpioReader& gpio_reader)`
- **Parameters:**
  - `gpio_reader`: Reference to the `GpioReader` instance handling hardware pin polling.
- **Return Value:** N/A (Constructor)
- **Main Function:** Initializes the Ground Station backend. It sets up button states, binds physical BCM pins via `GpioReader`, initializes networking/metrics, and launches the `worker_thread_` loop.

### `~GroundStation()`
- **Parameters:** None
- **Return Value:** N/A (Destructor)
- **Main Function:** Safely terminates the ground station by toggling the `running_` flag and joining the `worker_thread_` to ensure graceful background thread shutdown.

### `updateFSM()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Evaluates the `system_request_state_` and dispatches execution to the corresponding handler (`handle_state_init`, `handle_state_safe`, `handle_state_unsafe`, or `handle_state_abort`).

---

## State Handlers

### `handle_state_init()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Resets the sub-state to `UNSAFE_STATE_IDLE` and clears any pending active commands as the system enters INIT mode.

### `handle_state_safe()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Secures the state machine by resetting flags (`valveActivate`, `igniterActivate`), resetting to `UNSAFE_STATE_IDLE`, and terminating any pending commands.

### `handle_state_unsafe()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Routes execution to the appropriate unsafe sub-state function based on `unsafeState` (`unsafeIdle`, `unsafeFill`, `unsafeFire`, `unsafeValve`). Detects sub-state transitions and triggers `onUnsafeStateChanged`.

### `handle_state_abort()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Executes abort protocols by falling back to `UNSAFE_STATE_IDLE` and clearing all pending commands immediately.

---

## Unsafe Sub-state Handlers

### `unsafeIdle()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Monitors physical buttons while in idle. Dictates transitions into `UNSAFE_STATE_FILL` or `UNSAFE_STATE_FIRE` based on button combinations.

### `unsafeFire()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Manages the firing sequence. It verifies that both Engine and Fill Station are successfully in the `Ignite` state before permitting a transition to `UNSAFE_STATE_VALVE`. Defaults back to idle if arming is released.

### `unsafeValve()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Monitors the arm valve button. Reverts the system back to `UNSAFE_STATE_IDLE` if the button is released.

### `unsafeFill()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Allows valve commands while fill is engaged. Transitions back to `UNSAFE_STATE_IDLE` if the fill button is released.

---

## Threading & Processing Loop

### `run()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** The main worker thread loop executing at ~200Hz. Coordinates receiving network packets, executing the state machine processing, polling telemetry, and flushing outbound network queues.

### `processReceiving()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Drains incoming UDP traffic from the client and server interfaces. It tracks metrics, validates CRC-32 signatures, and pushes valid packet buffers to respective incoming thread-safe queues.

### `processStateMachine(std::chrono::steady_clock::time_point& last_timer)`
- **Parameters:**
  - `last_timer`: Reference to a steady_clock timestamp used to compute intervals for device timeouts.
- **Return Value:** `void`
- **Main Function:** The central logic orchestrator. Updates button debouncing, advances the FSM, handles 100ms command retries, increments device connection timeouts, and processes incoming network queues (routing ACK/State sync messages to Engine/Fill instances).

### `processSending()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Drains `server_outgoing_queue_` and `client_outgoing_queue_`, transmitting raw UDP packets through respective sockets and incrementing TX metrics.

### `tick()`
- **Parameters:** None
- **Return Value:** `bool`
- **Main Function:** Backward compatible routine to check if a UI redraw is required, clearing the internal `redraw_flag_`.

---

## System Request & Error Tracking

### `setErrorFlagged(bool error)`
- **Parameters:**
  - `error`: Boolean flag indicating if an error is present.
- **Return Value:** `void`
- **Main Function:** Updates `error_flagged_`, logs the status change, broadcasts the error to remote dashboard endpoints via `sendDeviceStatePacket`, and triggers UI redraw.

### `toggleErrorFlag()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Convenience wrapper that calls `setErrorFlagged` with the negated state of the current error flag.

### `setSystemRequestState(uint8_t state)`
- **Parameters:**
  - `state`: The requested logic control state (e.g., Safe, Unsafe, Abort).
- **Return Value:** `void`
- **Main Function:** Transitions the overall system state (blocking transitions if the physical Emergency Stop is engaged unless transitioning to Abort). Logs the transition and broadcasts the new requested state across the network.

---

## Hardware Input Control

### `updateButtons()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Iterates through all GPIO control buttons, enforces a 4ms debounce filter, reads raw physical states via `GpioReader`, and triggers `onButtonStateChanged` upon detected changes.

### `onButtonStateChanged(int index, bool pressed)`
- **Parameters:**
  - `index`: Array index representing the specific button.
  - `pressed`: True if pressed, false if released.
- **Return Value:** `void`
- **Main Function:** Routes hardware events to system behavior (e.g., triggering Abort upon emergency stop, or enabling/disabling Unsafe requests via key switch). Broadcasts updated ground station status to the network.

---

## Command Output & Network Packet Handling

### `sendDeviceStatePacket(uint8_t device_id, uint8_t payload_id, uint32_t state_val)`
- **Parameters:**
  - `device_id`: Targeted device identifier.
  - `payload_id`: Packet payload classification ID.
  - `state_val`: Requested state payload.
- **Return Value:** `void`
- **Main Function:** Assembles an `EthernetHeader` and dynamically allocates either a padded raw value or a `SetStateFrame`. Appends CRC-32 and enqueues the buffer for server transmission.

### `sendGSStatusPacket()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Constructs a `GSSystemState` struct mapped from current button logic states and ECU/FCU feedback, packages it into a standard telemetry packet, appends CRC, and enqueues for transmission.

### `sendTelemetryPacket()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Stub function meant to routinely construct and dispatch system-wide telemetry packets over UDP.

### `enqueueClientSend(const std::vector<uint8_t>& data)`
- **Parameters:**
  - `data`: Byte array representing a prepared UDP payload.
- **Return Value:** `void`
- **Main Function:** Pushes raw packet data into the thread-safe `client_outgoing_queue_`.

### `enqueueServerSend(const std::vector<uint8_t>& data)`
- **Parameters:**
  - `data`: Byte array representing a prepared UDP payload.
- **Return Value:** `void`
- **Main Function:** Pushes raw packet data into the thread-safe `server_outgoing_queue_`.

### `validateCrc(const std::vector<uint8_t>& data) const`
- **Parameters:**
  - `data`: Byte array of the received packet.
- **Return Value:** `bool`
- **Main Function:** Recalculates the CRC-32 signature of the packet payload minus the final 4 bytes, comparing it against the attached CRC tail block to verify data integrity.

---

## Unsafe State Commands & Sequence Tracking

### `getNextSeq()`
- **Parameters:** None
- **Return Value:** `uint8_t`
- **Main Function:** Safely fetches the next sequence number and increments `seq_counter_`, wrapping between 1 and 15 (0 is reserved for telemetry).

### `sendUnsafeCommand(uint8_t requested_state)`
- **Parameters:**
  - `requested_state`: The unsafe state to send (e.g., Ignite, Launch).
- **Return Value:** `void`
- **Main Function:** Wraps a state change into the pending command tracker struct, configures sequence/retries, and triggers the initial `transmitUnsafeCommand`.

### `transmitUnsafeCommand(uint8_t requested_state, uint8_t seq)`
- **Parameters:**
  - `requested_state`: The device state.
  - `seq`: The explicit sequence ID.
- **Return Value:** `void`
- **Main Function:** Explicitly maps parameters into a `SetStateFrame` and `EthernetHeader`, calculates CRC, and enqueues it to devices. Invoked initially by `sendUnsafeCommand` and recursively during retry intervals.

### `clearPendingCommand()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Deactivates the `pending_cmd_` tracker, stopping the 100ms retry loops when exiting unsafe constraints or achieving an ACK.

### `onUnsafeStateChanged(uint8_t prev, uint8_t current)`
- **Parameters:**
  - `prev`: Prior unsafe state.
  - `current`: New unsafe state.
- **Return Value:** `void`
- **Main Function:** Event hook triggered whenever the system transitions between Unsafe sub-states. Issues physical remote `Ignite` and `Launch` commands upon entering `UNSAFE_STATE_FIRE` or `UNSAFE_STATE_VALVE`.

---

## Utility & Logging

### `log(const std::string& message)`
- **Parameters:**
  - `message`: Text content to record.
- **Return Value:** `void`
- **Main Function:** Thread-safely appends logs to `logs_`, enforcing a 500-line history maximum limit, and triggers redraw updates.

### `getLogs()`
- **Parameters:** None
- **Return Value:** `std::vector<std::string>`
- **Main Function:** Safely returns a copy of the current system log history vector for UI rendering.

### `clearLogs()`
- **Parameters:** None
- **Return Value:** `void`
- **Main Function:** Empties the existing `logs_` vector, adds a clearance notice, and triggers UI redraw.
