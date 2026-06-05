#include "server.h"

#include <iostream>
#include <stdexcept>
#include <cstring>

// POSIX specific headers
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "config.h"

UdpServer::UdpServer() : socket_fd_(-1),port_(SERVER_PORT), status(SERVER_INIT), has_client_(false) {
    initSocket();
}

UdpServer::~UdpServer() {
    closeSocket();
}

bool UdpServer::receive(UdpMessage& out_msg) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) return false;

    uint8_t buffer[65507]; // Max UDP payload size
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    while (true) {
        ssize_t bytes_received = recvfrom(socket_fd_, buffer, sizeof(buffer), 0,
                                          (struct sockaddr*)&sender_addr, &sender_len);

        if (bytes_received > 0) {
            if (!has_client_) {
                client_addr_ = sender_addr;
                has_client_ = true;
                status = SERVER_CONNECTED;
                out_msg.data.assign(buffer, buffer + bytes_received);
                out_msg.sender_addr = sender_addr;
                return true;
            } else {
                if (sender_addr.sin_addr.s_addr == client_addr_.sin_addr.s_addr &&
                    sender_addr.sin_port == client_addr_.sin_port) {
                    out_msg.data.assign(buffer, buffer + bytes_received);
                    out_msg.sender_addr = sender_addr;
                    return true;
                } else {
                    // Ignore packet from other client
                    continue;
                }
            }
        } else {
            if (bytes_received < 0) {
                // EAGAIN or EWOULDBLOCK means no data is currently available
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::cerr << "UDP Receive error: " << strerror(errno) << std::endl;
                }
            }
            return false;
        }
    }
}

const ServerStatus UdpServer::getStatus()
{
    return status;
}

bool UdpServer::send(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) return false;

    ssize_t bytes_sent = sendto(socket_fd_, data.data(), data.size(), 0,
                                (const struct sockaddr*)&client_addr_, sizeof(client_addr_));

    if (bytes_sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // OS send buffer is full, try again later
            return false;
        }
        std::cerr << "UDP Send error: " << strerror(errno) << std::endl;
        return false;
    }

    return bytes_sent == static_cast<ssize_t>(data.size());
}

void UdpServer::initSocket() {
    // 1. Create UDP socket
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        status=  SERVER_ERROR;
        return;
    }

    // 2. Make the socket non-blocking
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    if (flags == -1) {
        closeSocket();
        status = SERVER_ERROR;
        return;
    }
    if (fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        closeSocket();
        status = SERVER_ERROR;
        return;
    }

    // 3. Bind the socket to the port
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port_);

    if (bind(socket_fd_, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        closeSocket();
        status=  SERVER_ERROR;
        return;
    }

    status = SERVER_LISTEN;
}

void UdpServer::closeSocket() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool UdpServer::isClientConnected() const {
    return has_client_;
}

std::string UdpServer::getConnectedClientIP() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (!has_client_) return "N/A";
    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(client_addr_.sin_addr), ip_str, INET_ADDRSTRLEN) == nullptr) {
        return "N/A";
    }
    return std::string(ip_str);
}

uint16_t UdpServer::getConnectedClientPort() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (!has_client_) return 0;
    return ntohs(client_addr_.sin_port);
}

void UdpServer::disconnectClient() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (has_client_) {
        has_client_ = false;
        std::memset(&client_addr_, 0, sizeof(client_addr_));
        if (status == SERVER_CONNECTED) {
            status = SERVER_LISTEN;
        }
    }
}