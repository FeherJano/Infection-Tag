#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once
#include <vector>
#include <queue>
#include <chrono>
#include <iostream>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../MessageTypes.hpp"
#include "../../task.hpp"
#include "../../map.hpp"
#include <unordered_map>


using json = nlohmann::json;

enum serverState { serverStateIdle, serverStateLobby, serverStateGameStart, serverStateGame };

struct PlayerInfo {
    std::string role; // "Survivor" vagy "Killer"
    sf::Vector2f position;
};

class CatGameServer {
public:
    CatGameServer(asio::io_context& ioContext, uint16_t port);
    ~CatGameServer();
    void ServerFunction();
    void setState(serverState newState);
    //void startGame();
    void broadcastGameData(const json& customMessage);
    void broadcastGameData();
    void generateGameData();
    json gameData;
    serverState currentState;

    std::unordered_map<std::string, std::string> playerRoles; // "Killer" vagy "Survivor"
    void processPlayerInput(const std::string& playerId, const sf::Vector2f& direction);
    std::string getPlayerIdByEndpoint(const asio::ip::udp::endpoint& endpoint);
    bool checkEndgameCondition();

    bool reset();

private:
    asio::ip::udp::socket socket;
    std::unordered_map<std::string, asio::ip::udp::endpoint> players;
    std::unordered_map<std::string, asio::ip::udp::endpoint> playersEndpoints;

    std::vector<Survivor> survivors;
    Killer killer;
    std::vector<std::vector<int>> maze;
    std::vector<Task> tasks;

    std::chrono::steady_clock::time_point gameStartTime;

    std::vector<std::vector<std::pair<int, int>>> compressMap(const std::vector<std::vector<int>>& map);

    void listen();
    void setupGameState();
    int playerCounter;
    
};

#endif
