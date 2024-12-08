#pragma once
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace messageSet {
    inline const std::string OK = "OK";
    inline const std::string NOK = "NOK";
    inline const std::string invalidOP = "invalidOp";
    inline const std::string connReq = "connReq";
    inline const std::string connAbort = "connAb";
    inline const std::string clientReady = "cReady"; // Client is ready to start the game
    inline const std::string gameStart = "start";
    inline const std::string gameState = "gameState"; // Új tag
};

namespace msgTypes {
    inline const std::string msgType = "msg_type"; // Each message has a message type
    inline const std::string mapData = "mapData";
    inline const std::string playerData = "pData";
    inline const std::string playerId = "pId";
}
