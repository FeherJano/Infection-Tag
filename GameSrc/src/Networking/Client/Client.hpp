#pragma once
#include <array>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Player/player.hpp"
#include "../MessageTypes.hpp"

using asio::ip::udp;
using json = nlohmann::json;

enum clientState{cStateMenu,cStateWaitGame,cStateStartGame,cStateRunGame,cStateExit};

class Client {
public:
	const static uint32_t maxMessageLength = 2048;
	static uint8_t maxRetries;

	Client(const std::string& address, uint16_t port,asio::io_context& ioC);
	~Client() = default;
	clientState getState()const;
	void setState(clientState newState);
	std::string connect();
	bool msgToServer(json &message);
	json msgFromServer();
	void waitForGame();
	void sendReady(bool ready);
	void sendDisconnect();

private:
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