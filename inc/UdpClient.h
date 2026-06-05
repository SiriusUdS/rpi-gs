#pragma once

#include <vector>
#include <mutex>
#include <cstdint>
#include <string>
#include <netinet/in.h>

typedef enum ClientStatus {
    CLIENT_INIT = 0,
    CLIENT_READY,
    CLIENT_ERROR,
    CLIENT_CLOSED
} ClientStatus;

struct UdpClientMessage {
    std::vector<uint8_t> data;
    struct sockaddr_in sender_addr;
};

class UdpClient {
public:
    UdpClient(const std::string& server_ip, uint16_t server_port, uint16_t local_port = 0);
    ~UdpClient();

    // Prevent copy
    UdpClient(const UdpClient&) = delete;
    UdpClient& operator=(const UdpClient&) = delete;

    /**
     * @brief Attempts to receive a message. Non-blocking.
     * @param out_msg The message object to populate if data is received.
     * @return true if a message was received, false if no data is available.
     */
    bool receive(UdpClientMessage& out_msg);

    /**
     * @brief Attempts to send raw bytes to the configured server. Non-blocking.
     * @param data The raw bytes to send.
     * @return true if successful, false otherwise.
     */
    bool send(const std::vector<uint8_t>& data);

    ClientStatus getStatus();

private:
    std::string server_ip_;
    uint16_t server_port_;
    uint16_t local_port_;
    int socket_fd_;
    std::mutex socket_mutex_; // Thread safety
    ClientStatus status_;
    struct sockaddr_in server_addr_;

    void initSocket();
    void closeSocket();
};
