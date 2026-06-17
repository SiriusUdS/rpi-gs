#pragma once

#include <vector>
#include <mutex>
#include <cstdint>
#include <atomic>
#include <string>
#include <functional>
#include <netinet/in.h> // Required for struct sockaddr_in

enum ServerStatus {
    SERVER_INIT = 0,
    SERVER_LISTEN,
    SERVER_CONNECTED,
    SERVER_ERROR,
    SERVER_CLOSED
};

// Struct to hold raw byte messages and the sender's address for replying
struct UdpMessage {
    std::vector<uint8_t> data;
    struct sockaddr_in sender_addr;
};

class UdpServer {
public:
    UdpServer();
    ~UdpServer();

    // Prevents copying of the server object
    UdpServer(const UdpServer&) = delete;
    UdpServer& operator=(const UdpServer&) = delete;

    /**
     * @brief Attempts to receive a message. Non-blocking.
     * @param out_msg The message object to populate if data is received.
     * @return true if a message was received, false if no data is available.
     */
    bool receive(UdpMessage& out_msg);

    const ServerStatus getStatus();

    /**
     * @brief Attempts to send raw bytes to a specific destination. Non-blocking.
     * @param data The raw bytes to send.
     * @return true if successful, false otherwise.
     */
    bool send(const std::vector<uint8_t>& data);

    // Client connection getters
    bool isClientConnected() const;
    std::string getConnectedClientIP();
    uint16_t getConnectedClientPort();
    void disconnectClient();

    // Set callback for routing logs
    void setLogCallback(std::function<void(const std::string&)> cb) { log_callback_ = cb; }

private:
    uint16_t port_;
    int socket_fd_;
    std::mutex socket_mutex_; // Ensures thread-safe access to the socket
    std::atomic<ServerStatus> status;
    std::atomic<bool> has_client_;
    struct sockaddr_in client_addr_;
    std::function<void(const std::string&)> log_callback_;

    void initSocket();
    void closeSocket();
};