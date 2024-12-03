#pragma once
#include <string>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace messageSet {
	extern const std::string OK;
	extern const std::string NOK;
	extern const std::string invalidOP;
	extern const std::string connReq; 
	extern const std::string connAbort;
	extern const std::string clientReady; //client is ready to start the game
	extern const std::string gameStart;
};

namespace msgTypes {
	extern const std::string msgType; //each message has a message type
	extern const std::string mapData;
	extern const std::string playerData;
	extern const std::string playerId;
	
}



