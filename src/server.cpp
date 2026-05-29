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

UdpServer::UdpServer() : socket_fd_(-1),port_(SERVER_PORT), status(SERVER_INIT) {
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

    ssize_t bytes_received = recvfrom(socket_fd_, buffer, sizeof(buffer), 0,
                                      (struct sockaddr*)&sender_addr, &sender_len);

    if (bytes_received > 0) {
        out_msg.data.assign(buffer, buffer + bytes_received);
        out_msg.sender_addr = sender_addr;
        return true;
    } else if (bytes_received < 0) {
        // EAGAIN or EWOULDBLOCK means no data is currently available
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "UDP Receive error: " << strerror(errno) << std::endl;
        }
    }
    
    return false;
}

const ServerStatus UdpServer::getStatus()
{
    return status;
}

bool UdpServer::send(const std::vector<uint8_t>& data, const struct sockaddr_in& dest_addr) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) return false;

    ssize_t bytes_sent = sendto(socket_fd_, data.data(), data.size(), 0,
                                (const struct sockaddr*)&dest_addr, sizeof(dest_addr));

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