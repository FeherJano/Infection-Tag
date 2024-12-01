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
	const static uint32_t maximumMessageLength;
	const static uint16_t defaultPort;
	const static uint8_t maxPlayers;

	CatGameServer(asio::io_context& ioC, uint16_t desiredPort = CatGameServer::defaultPort);
	~CatGameServer();

	void setState(serverState newState);
	serverState getCurrentState()const;
	void setPlayerCount(uint8_t pc);
	uint8_t getPlayerCount()const;

	bool broadcastMessage(json msg);
	bool sendMsg(udp::endpoint to, json msg);
	void shutDown();
	void ServerFunction();

private:
	uint8_t playerCount;
	serverState currentState;
	udp::socket mainSocket;
	std::unordered_map<std::string, std::pair<udp::endpoint,std::queue<json>>> players; //hasmap - key: playerId, value: a pair of adress and message queue of the player 
	void listen();
	std::string registerPlayer(udp::endpoint playerAddress);
	bool startGame();
	

};