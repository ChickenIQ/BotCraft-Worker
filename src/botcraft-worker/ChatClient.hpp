#pragma once

#include <botcraft/Game/ManagersClient.hpp>
#include <botcraft/Renderer/RenderingManager.hpp>

#include "SocketPacket.hpp"
#include "AtomicQueue.hpp"

class ChatClient : public Botcraft::ManagersClient {
    std::atomic_bool alive;
    int8_t id;
    const std::string address, username;
    AtomicQueue<SocketPacket> rx, tx;
    bool isOnline;

public:
    ChatClient(int8_t id, std::string addr, std::string user, bool isOnline = true);

    ~ChatClient() override;

    void Connect();

    std::optional<SocketPacket> PopPacket();

    int8_t GetId() const;

protected:
    void Handle(ProtocolCraft::ClientboundDisconnectPacket &msg) override;

    void Handle(ProtocolCraft::ClientboundSystemChatPacket &msg) override;

    void Handle(ProtocolCraft::ClientboundPlayerChatPacket &msg) override;

    void Handle(ProtocolCraft::ClientboundDisguisedChatPacket &msg) override;
};
