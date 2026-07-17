#include <botcraft/Utilities/Logger.hpp>
#include "botcraft-worker/ChatClient.hpp"

#ifndef BOTCRAFT_WORKER_FLATTEN_PLAYERCHAT
#define BOTCRAFT_WORKER_FLATTEN_PLAYERCHAT true
#endif

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
    LOG_DEBUG(id << ": " << str);
}

void ChatClient::Handle(ProtocolCraft::ClientboundPlayerChatPacket &msg) {
    if constexpr (BOTCRAFT_WORKER_FLATTEN_PLAYERCHAT) {
        std::string uc = msg.GetBody().GetContent();
        if (uc.empty()) return;
        const std::string userName = this->GetPlayerName(msg.GetSender());
        uc = "<" + userName + "> " + uc;
        tx.push(SocketPacket_MakeChatPacket(id, uc));
        LOG_DEBUG(id << ": " << uc);
    } else {
        const std::string uc = msg.GetBody().GetContent();
        if (uc.empty()) return;
        tx.push(SocketPacket_MakePlayerChatPacket(id, msg.GetSender(), uc));
        LOG_DEBUG(id << ": " << uc);
    }
}

void ChatClient::Handle(ProtocolCraft::ClientboundDisguisedChatPacket &msg) {
    auto str = msg.GetMessage().GetText();
    if (str.empty()) return;
    tx.push(SocketPacket_MakeChatPacket(id, str));
    LOG_DEBUG(id << ": " << str);
}
