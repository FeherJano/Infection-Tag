#pragma once
#include <vector>
#include <queue>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Utility/logging.hpp"
#include "../MessageTypes.hpp"

using json = nlohmann::json;
using asio::ip::udp;

enum serverState{serverStateLobby,serverStateGameStart,serverStateGameRun,serverStateIdle, serverStateExit};

const uint32_t maximumMessageLength = 2048;
class CatGameServer{
public:	
	
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
	json getMsg(udp::endpoint &from);
	void shutDown();
	void ServerFunction();

private:
	uint8_t playerCount;
	uint8_t readyPlayers;
	serverState currentState;
	udp::socket mainSocket;
	std::array<char, maximumMessageLength> recvBuf;
	std::unordered_map<std::string, std::pair<udp::endpoint,std::queue<json>>> players; //hasmap - key: playerId, value: a pair of adress and message queue of the player 
	void listen();
	std::string registerPlayer(udp::endpoint playerAddress);
	bool startGame();
	void handlePlayer(std::string playerID);

	

};