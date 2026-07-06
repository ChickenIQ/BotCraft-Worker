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
    if (!this->GetShouldBeClosed()) {
        if (bool expected = false; !alive.compare_exchange_strong(expected, true)) return;
    }
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
    auto str = msg.GetContent().GetText();
    if (str.empty()) return;
    tx.push(SocketPacket_MakeChatPacket(id, str));
}

void ChatClient::Handle(ProtocolCraft::ClientboundPlayerChatPacket &msg) {
    auto uc = msg.GetUnsignedContent();
    if (!uc || uc->GetText().empty()) return;
    tx.push(SocketPacket_MakeChatPacket(id, uc->GetText()));
}

void ChatClient::Handle(ProtocolCraft::ClientboundDisguisedChatPacket &msg) {
    auto str = msg.GetMessage().GetText();
    if (str.empty()) return;
    tx.push(SocketPacket_MakeChatPacket(id, str));
}
