#include <botcraft/Utilities/Logger.hpp>
#include "botcraft-worker/ChatClient.hpp"

ChatClient::ChatClient(const int8_t id, std::string addr, std::string user, bool const isOnline) : alive(false),
    id(id),
    address(std::move(addr)),
    username(std::move(user)),
    isOnline(isOnline) {
}

ChatClient::~ChatClient() {
    alive = false;
    this->ManagersClient::Disconnect();
}

void ChatClient::Connect() {
    if (bool expected = false; !alive.compare_exchange_strong(expected, true)) return;
    if (isOnline)
        this->ConnectMicrosoft(this->address, this->username);
    else
        this->ManagersClient::Connect(this->address, this->username);
    this->SetAutoRespawn(true);
}

std::optional<SocketPacket> ChatClient::PopPacket() {
    if (tx.size() == 0) return std::nullopt;
    return tx.pull();
}

int8_t ChatClient::GetId() const {
    return id;
}

void ChatClient::Handle(ProtocolCraft::ClientboundDisconnectPacket &msg) {
    alive = false;
}

void ChatClient::Handle(ProtocolCraft::ClientboundSystemChatPacket &msg) {
    const std::istringstream ss{msg.GetContent().GetText()};
    tx.push(SocketPacket_MakeChatPacket(id, ss.str()));
}

void ChatClient::Handle(ProtocolCraft::ClientboundPlayerChatPacket &msg) {
    const std::istringstream ss{msg.GetBody().GetContent()};
    tx.push(SocketPacket_MakeChatPacket(id, ss.str()));
}
