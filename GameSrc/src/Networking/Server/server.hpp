#pragma once
#include <vector>
#include <queue>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Utility/logging.hpp"
#include "../MessageTypes.hpp"


using nlohmann::json;
using asio::ip::udp;

enum serverState{serverStateLobby,serverStateGameStart,serverStateGameRun,serverStateIdle};

                
class CatGameServer{
public:
	const static uint16_t defaultPort;
	const static unsigned maximumMessageLength;
	const static uint8_t maxPlayers;

	CatGameServer(asio::io_context& ioC, uint16_t desiredPort = CatGameServer::defaultPort);
	~CatGameServer();

	void setState(serverState newState);
	serverState getCurrentState()const;
	void setPlayerCount(uint8_t pc);
	uint8_t getPlayerCount()const;

	void ServerFunction();
	bool broadcastMessage(json msg);
	bool sendMsg(udp::endpoint to, json msg);

private:
	uint8_t playerCount;
	serverState currentState;
	udp::socket mainSocket;
	messageSet aviableMessages;
	std::unordered_map<std::string, std::queue<json>> messageQueue; //hasmap - key: playerId, value: message queue for that player

	void listen();
	bool registerPlayer();
	

};