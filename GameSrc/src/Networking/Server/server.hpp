#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once
#include <vector>
#include <queue>
#include <chrono>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Utility/logging.hpp"
#include "../MessageTypes.hpp"
#include "../../task.hpp"
#include "../../map.hpp"
#include <unordered_map>


using json = nlohmann::json;

enum serverState { serverStateIdle, serverStateLobby, serverStateGameStart, serverStateGame };

class CatGameServer {
public:
    CatGameServer(asio::io_context& ioContext, uint16_t port);
    void ServerFunction();
    void setState(serverState newState);
    //void startGame();
    void broadcastGameData();
    void generateGameData();
    json gameData;
    serverState currentState;

private:
    asio::ip::udp::socket socket;
    std::unordered_map<std::string, asio::ip::udp::endpoint> players;
    std::unordered_map<std::string, asio::ip::udp::endpoint> playersEndpoints;
    

    void listen();
    void setupGameState();

    
};

#endif
