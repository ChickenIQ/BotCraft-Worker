// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef
#include <cstdint>
#include <cstring>
#include <string>

#include "botcraft-worker/SocketPacket.hpp"
#include "botcraft-worker/ChatClient.hpp"

SocketPacket SocketPacket_MakeChatPacket(const int8_t botId, const std::string &msg) {
    // ReSharper disable once CppLocalVariableMayBeConst
    SocketPacket packet = {
        .type = SOCKET_PACKET_TYPE_CHAT,
        .id = botId,
        .length = static_cast<uint16_t>(msg.length() > sizeof(packet.payload) ? sizeof(packet.payload) : msg.length()),
        .payload = ""
    };
    strncpy((char *) packet.payload, msg.c_str(), sizeof(packet.payload));
    return packet;
}

SocketPacket SocketPacket_MakeInfoPacket(const std::vector<std::string> &bots) {
    // ReSharper disable once CppLocalVariableMayBeConst
    SocketPacket packet = {
        .type = SOCKET_PACKET_TYPE_INFO,
        .id = 0,
        .length = static_cast<uint16_t>(bots.size() * SOCKET_PACKET_PAYLOAD_USIZE > sizeof(packet.payload)
                                            ? sizeof(packet.payload)
                                            : bots.size() * SOCKET_PACKET_PAYLOAD_USIZE
        ),
        .payload = ""
    };
    for (uint_fast16_t i = 0; i < packet.length; i += SOCKET_PACKET_PAYLOAD_USIZE)
        strncpy((char *) packet.payload + i, bots[i / SOCKET_PACKET_PAYLOAD_USIZE].c_str(),
                SOCKET_PACKET_PAYLOAD_USIZE);
    return packet;
}

SocketPacket SocketPacket_MakeDisconnectPacket(const int8_t botId, const std::string &reason) {
    // ReSharper disable once CppLocalVariableMayBeConst
    SocketPacket packet = {
        .type = SOCKET_PACKET_TYPE_DISCONNECT,
        .id = botId,
        .length = static_cast<uint16_t>(reason.length() > sizeof(packet.payload) ? sizeof(packet.payload) : reason.length()),
        .payload = ""
    };
    strncpy((char *) packet.payload, reason.c_str(), sizeof(packet.payload));
    return packet;
}

SocketPacket SocketPacket_MakePlayerChatPacket(const int8_t botId, std::array<unsigned char, SOCKET_PACKET_PAYLOAD_USIZE> uuid, const std::string &msg) {
    // ReSharper disable once CppLocalVariableMayBeConst
    SocketPacket packet = {
        .type = SOCKET_PACKET_TYPE_PCHAT,
        .id = botId,
        .length = static_cast<uint16_t>(msg.length() + SOCKET_PACKET_PAYLOAD_USIZE > sizeof(packet.payload) ? 
            sizeof(packet.payload) : msg.length() + SOCKET_PACKET_PAYLOAD_USIZE),
        .payload = ""
    };
    memcpy((char *) packet.payload, uuid.data(), SOCKET_PACKET_PAYLOAD_USIZE);
    strncpy((char *) packet.payload + SOCKET_PACKET_PAYLOAD_USIZE, msg.c_str(), sizeof(packet.payload) - SOCKET_PACKET_PAYLOAD_USIZE);
    return packet;
}

void PacketConsume_Chat(void *c, void *p) {
    auto *client = static_cast<ChatClient *>(c);
    auto *packet = static_cast<SocketPacket *>(p);
    if (packet->length > sizeof(SocketPacket::payload)) return;

    auto str = static_cast<char *>(alloca(packet->length + 1));
    strncpy(str, reinterpret_cast<char *>(packet->payload), packet->length);
    str[packet->length] = '\0';
    client->SendChatMessage(str);
}

void PacketConsume_Disconnect(void *c, void *) {
    auto *client = static_cast<ChatClient *>(c);
    client->Disconnect();
}
