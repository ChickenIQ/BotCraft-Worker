#include <utility>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <botcraft/Utilities/Logger.hpp>

#include "botcraft-worker/SocketConnection.hpp"

SocketConnection::SocketConnection(std::string sockName) : alive(false), sockName(std::move(sockName)), fd(-1) {
}

SocketConnection::~SocketConnection() {
    if (!alive) return;
    Disconnect();
}

void SocketConnection::Connect() {
    if (bool expected = false; !alive.compare_exchange_strong(expected, true)) return;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        alive = false;
        LOG_ERROR("Unable to open client socket");
        return;
    }

    sockaddr_un server_addr = {};
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, sockName.c_str(), sizeof(server_addr.sun_path) - 1);

    if (connect(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
        close(fd);
        alive = false;
        LOG_ERROR("Socket connection failed");
    }
}

void SocketConnection::Disconnect() {
    if (!alive) return;

    shutdown(fd, SHUT_RDWR);
    close(fd);
    alive = false;
}

void SocketConnection::QueuePacket(const SocketPacket &packet) {
    tx.push(packet);
}

bool SocketConnection::ReceivePacket(SocketPacket &packet) {
    if (rx.size() == 0) return false;
    packet = rx.pull();
    return true;
}

void SocketConnection::ProcessQueue() {
    while (alive && tx.size() > 0) {
        if (SocketPacket packet = tx.peek(); !ProcessSinglePacket(packet)) break;
        tx.pop();
    }

    ProcessIncomingPackets();
}

bool SocketConnection::ProcessIncomingPackets() {
    int avail = 0;
    if (ioctl(fd, FIONREAD, &avail) == -1) {
        Disconnect();
        return false;
    }

    avail -= avail % static_cast<int>(sizeof(SocketPacket));
    for (int i = 0; i < avail / sizeof(SocketPacket); i++) {
        SocketPacket packet;
        if (read(fd, &packet, sizeof(SocketPacket)) != sizeof(SocketPacket)) {
            LOG_ERROR("Failed to read whole packet from socket, reconnecting");
            Disconnect();
            return false;
        }
        rx.push(packet);
    }

    return true;
}

bool SocketConnection::ProcessSinglePacket(const SocketPacket &packet) {
    if (send(fd, &packet, sizeof(packet), 0) != sizeof(packet)) {
        LOG_ERROR("Failed to send whole packet to socket, reconnecting");
        Disconnect();
        return false;
    }
    return true;
}
