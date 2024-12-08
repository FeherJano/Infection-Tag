#pragma once
#include <array>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Player/player.hpp"
#include "../MessageTypes.hpp"
#include <functional>

using asio::ip::udp;
using json = nlohmann::json;
using GameStateCallback = std::function<void(const json&)>;

enum clientState{cStateMenu,cStateWaitGame,cStateStartGame,cStateRunGame,cStateExit};

class Client {
public:
	const static uint32_t maxMessageLength = 8192;
	static uint8_t maxRetries;

	Client(const std::string& address, uint16_t port,asio::io_context& ioC);
	~Client() = default;
	clientState getState()const;
	void setState(clientState newState);
	std::string connect();
	bool msgToServer(json &message);
	json msgFromServer();
	void setGameStateCallback(GameStateCallback callback);
	void waitForGame();
	void sendReady(bool ready);
	void sendDisconnect();

private:
	GameStateCallback gameStateCallback; // A callback t�rol�sa
	

	clientState currentState;
	uint16_t port;
	udp::endpoint remoteSendEndp;
	udp::endpoint remoteRecieveEndp;
	udp::socket mainSocket;
	std::array<char, maxMessageLength> recvBuf;
	std::array<char, maxMessageLength> sendBuf;
	std::string playerId;
	Killer* myKiller;
	Survivor* mySurvivor;

};