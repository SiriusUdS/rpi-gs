#pragma once

#include <vector>
#include <mutex>
#include <cstdint>
#include <netinet/in.h> // Required for struct sockaddr_in


typedef enum ServerStatus{
    SERVER_INIT=0,
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
     * @param dest_addr The destination address.
     * @return true if successful, false otherwise.
     */
    bool send(const std::vector<uint8_t>& data, const struct sockaddr_in& dest_addr);

private:
    uint16_t port_;
    int socket_fd_;
    std::mutex socket_mutex_; // Ensures thread-safe access to the socket
    ServerStatus status;

    void initSocket();
    void closeSocket();
};