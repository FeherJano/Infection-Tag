#ifndef CLIENT_HPP
#define CLIENT_HPP

#pragma once
#include <array>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Player/player.hpp"
#include "../MessageTypes.hpp"
#include <functional>


using json = nlohmann::json;
using namespace std;

class Client {
public:
    Client(const std::string& serverAddress, uint16_t port, asio::io_context& ioContext);
    std::string connect();
    void waitForGameData();

private:
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint serverEndpoint;
};

#endif
