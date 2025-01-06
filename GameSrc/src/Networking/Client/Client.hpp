#ifndef CLIENT_HPP
#define CLIENT_HPP

#pragma once
#include <array>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Player/player.hpp"
#include "../MessageTypes.hpp"
#include "../../task.hpp"
#include <functional>


using json = nlohmann::json;
using namespace std;

class Client {
public:
    Client(const std::string& serverAddress, uint16_t port, asio::io_context& ioContext);
    std::string connect();
    void waitForGameData();

    json getGameData() const { return gameData; }
    bool isGameDataReady() const { return gameDataReady; }
    Player* getPlayer() { return player.get(); }

    std::string getId() const { return playerId; }
    void setId(const std::string& id) { playerId = id; }

    std::vector<std::vector<int>> decompressMap(const std::vector<std::vector<std::pair<int, int>>>& compressedMap);

    void sendPlayerInput(const sf::Vector2f& direction);


private:
    json gameData;
    bool gameDataReady = false;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint serverEndpoint;
    std::unique_ptr<Player> player;
    std::string playerId;
    
};

#endif
