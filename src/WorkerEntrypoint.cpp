#include <string>
#include <thread>
#include <csignal>
#include <atomic>
#include <botcraft/Network/NetworkManager.hpp>
#include <botcraft/Utilities/Logger.hpp>

std::string getEnvVar(std::string const &key);

std::vector<std::string> splitByDelimiter(const std::string &full, char delimiter = ',');

void signalHandler(int signum);

#include "botcraft-worker/SocketPacket.hpp"
#include "botcraft-worker/AtomicQueue.hpp"
#include "botcraft-worker/ChatClient.hpp"
#include "botcraft-worker/SocketConnection.hpp"

std::atomic_bool worker_running(true);

int main() {
    std::signal(SIGINT, signalHandler);

    Botcraft::Logger::GetInstance().SetLogLevel(Botcraft::LogLevel::Info);
    Botcraft::Logger::GetInstance().SetFilename("");
    Botcraft::Logger::GetInstance().RegisterThread("main");

    std::string serverAddress = getEnvVar("SERVER_ADDRESS");
    std::string microsoftLogins = getEnvVar("MICROSOFT_USERNAMES");
    std::string offlineLogins = getEnvVar("OFFLINE_USERNAMES");
    std::string socketPath = getEnvVar("SOCKET_PATH");
    std::string logLevelString = getEnvVar("LOG_LEVEL");

    if (serverAddress.empty()) serverAddress = "127.0.0.1";
    if (socketPath.empty()) socketPath = "/tmp/botcraft-worker.sock";
    if (microsoftLogins.empty() && offlineLogins.empty()) {
        LOG_FATAL("Must have at least one online or offline account");
        return -1;
    }

    const std::vector<std::string> microsoftUsernames = splitByDelimiter(microsoftLogins);
    const std::vector<std::string> offlineUsernames = splitByDelimiter(offlineLogins);

    try {
        try {
            const int logLevel = std::stoi(logLevelString);
            Botcraft::Logger::GetInstance().SetLogLevel(static_cast<Botcraft::LogLevel>(logLevel));
        } catch (std::exception &) {
        }

        std::vector<ChatClient *> clients;
        auto *connection = new SocketConnection(socketPath);

        for (int i = 0; i < microsoftUsernames.size(); i++) {
            clients.push_back(new ChatClient(static_cast<int8_t>(i + 1), serverAddress, microsoftUsernames[i], true));
        }

        for (int i = 0; i < offlineUsernames.size(); i++) {
            clients.push_back(new ChatClient(static_cast<int8_t>(i + 1), serverAddress, offlineUsernames[i], false));
        }

        for (auto client: clients) client->Connect();

        std::vector<std::string> usernames;
        usernames.reserve(clients.size());
        for (ChatClient *client: clients) {
            std::shared_ptr<Botcraft::NetworkManager> nm = client->GetNetworkManager();
            std::string username = nm.get()->GetMyName();
            usernames.push_back(username);
        }
        connection->QueuePacket(SocketPacket_MakeInfoPacket(usernames));

        while (worker_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            connection->Connect();
            for (ChatClient *client: clients) {
                client->Connect();
                while (true) {
                    if (const std::optional<SocketPacket> pck = client->PopPacket(); pck.has_value()) {
                        connection->QueuePacket(pck.value());
                    } else break;
                }
            }
            connection->ProcessQueue();
            while (true) {
                SocketPacket inbound;
                if (!connection->ReceivePacket(inbound)) break;
                if (inbound.type >= PacketConsumeHandlers.size()) continue;
                if (inbound.type == SOCKET_PACKET_TYPE_STOP) {
                    worker_running = false;
                    continue;
                }
                const PacketHandler callableFunction = PacketConsumeHandlers[inbound.type];
                if (callableFunction == nullptr) continue;
                for (ChatClient *client: clients) {
                    const bool shouldCall = inbound.id == 0 || client->GetId() == inbound.id || (
                                                inbound.id < 0 && abs(inbound.id) != client->GetId());
                    if (shouldCall) {
                        callableFunction(client, &inbound);
                        if (inbound.id != 0) break;
                    }
                }
            }
        }

        for (ChatClient *client: clients) delete client;
        clients.clear();
        delete connection;

        LOG_INFO("Goodbye!");
        return 0;
    } catch (std::exception &e) {
        LOG_FATAL("Exception: " << e.what());
        return 1;
    } catch (...) {
        LOG_FATAL("Unknown exception");
        return 2;
    }
}

std::string getEnvVar(std::string const &key) {
    char *val = getenv(key.c_str());
    return val == nullptr ? std::string("") : std::string(val);
}

std::vector<std::string> splitByDelimiter(const std::string &full, const char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(full);
    while (std::getline(tokenStream, token, delimiter)) tokens.push_back(token);
    return tokens;
}

void signalHandler(int signum) {
    worker_running = false;
}
