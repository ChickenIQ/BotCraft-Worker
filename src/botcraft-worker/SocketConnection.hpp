#pragma once
#include <atomic>
#include <string>

#include "AtomicQueue.hpp"
#include "SocketPacket.hpp"

class SocketConnection {
    std::atomic_bool alive;
    std::string sockName;
    AtomicQueue<SocketPacket> rx, tx;
    std::atomic<int> fd;

public:
    explicit SocketConnection(std::string sockName);

    ~SocketConnection();

    void Connect();

    void Disconnect();

    void QueuePacket(const SocketPacket &packet);

    bool ReceivePacket(SocketPacket &packet);

    void ProcessQueue();

protected:
    bool ProcessSinglePacket(const SocketPacket &packet);

    bool ProcessIncomingPackets();
};
