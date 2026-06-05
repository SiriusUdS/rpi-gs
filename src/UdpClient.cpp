#include "UdpClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

UdpClient::UdpClient(const std::string& server_ip, uint16_t server_port, uint16_t local_port)
    : server_ip_(server_ip), server_port_(server_port), local_port_(local_port),
      socket_fd_(-1), status_(CLIENT_INIT) {
    initSocket();
}

UdpClient::~UdpClient() {
    closeSocket();
}

bool UdpClient::receive(UdpClientMessage& out_msg) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) return false;

    uint8_t buffer[65507]; // Max UDP payload size
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    ssize_t bytes_received = recvfrom(socket_fd_, buffer, sizeof(buffer), 0,
                                      (struct sockaddr*)&sender_addr, &sender_len);

    if (bytes_received > 0) {
        out_msg.data.assign(buffer, buffer + bytes_received);
        out_msg.sender_addr = sender_addr;
        return true;
    } else if (bytes_received < 0) {
        // EAGAIN or EWOULDBLOCK means no data is currently available
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "UDP Client Receive error: " << strerror(errno) << std::endl;
        }
    }
    
    return false;
}

bool UdpClient::send(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) return false;

    ssize_t bytes_sent = sendto(socket_fd_, data.data(), data.size(), 0,
                                (const struct sockaddr*)&server_addr_, sizeof(server_addr_));

    if (bytes_sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // OS send buffer is full, try again later
            return false;
        }
        std::cerr << "UDP Client Send error: " << strerror(errno) << std::endl;
        return false;
    }

    return bytes_sent == static_cast<ssize_t>(data.size());
}

ClientStatus UdpClient::getStatus() {
    return status_;
}

void UdpClient::initSocket() {
    // 1. Create UDP socket
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        status_ = CLIENT_ERROR;
        return;
    }

    // 2. Make the socket non-blocking
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    if (flags == -1) {
        closeSocket();
        status_ = CLIENT_ERROR;
        return;
    }
    if (fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        closeSocket();
        status_ = CLIENT_ERROR;
        return;
    }

    // 3. Bind the socket to the local port (if specified)
    if (local_port_ > 0) {
        struct sockaddr_in local_addr;
        std::memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
        local_addr.sin_port = htons(local_port_);

        if (bind(socket_fd_, (const struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            closeSocket();
            status_ = CLIENT_ERROR;
            return;
        }
    }

    // 4. Configure remote server address
    std::memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(server_port_);
    if (inet_pton(AF_INET, server_ip_.c_str(), &server_addr_.sin_addr) <= 0) {
        closeSocket();
        status_ = CLIENT_ERROR;
        return;
    }

    status_ = CLIENT_READY;
}

void UdpClient::closeSocket() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
        status_ = CLIENT_CLOSED;
    }
}
