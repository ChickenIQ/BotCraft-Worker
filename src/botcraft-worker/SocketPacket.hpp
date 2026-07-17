#pragma once

#define SOCKET_PACKET_PAYLOAD_SIZE  32768
#define SOCKET_PACKET_PAYLOAD_USIZE 16

#define SOCKET_PACKET_TYPE_CHAT         0
#define SOCKET_PACKET_TYPE_INFO         1
#define SOCKET_PACKET_TYPE_STOP         2
#define SOCKET_PACKET_TYPE_DISCONNECT   3
#define SOCKET_PACKET_TYPE_PCHAT        4

#include <functional>
#include <vector>
#include <cstdint>

typedef struct {
    uint8_t type;
    int8_t id;
    uint16_t length;
    uint8_t payload[SOCKET_PACKET_PAYLOAD_SIZE];
} __attribute__((packed)) SocketPacket;

void PacketConsume_Chat(void *, void *);

void PacketConsume_Disconnect(void *, void *);

using PacketHandler = std::function<void(void *, void *)>;
static std::array<PacketHandler, 4> PacketConsumeHandlers = {
    PacketConsume_Chat,
    nullptr,
    nullptr,
    PacketConsume_Disconnect,
};

SocketPacket SocketPacket_MakeChatPacket(int8_t botId, const std::string &msg);

SocketPacket SocketPacket_MakeInfoPacket(const std::vector<std::string> &bots);

SocketPacket SocketPacket_MakeDisconnectPacket(const int8_t botId, const std::string &reason);

SocketPacket SocketPacket_MakePlayerChatPacket(const int8_t botId, std::array<unsigned char, SOCKET_PACKET_PAYLOAD_USIZE> uuid, const std::string &msg);
