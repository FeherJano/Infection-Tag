#pragma once
#include <array>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Player/player.hpp"
#include "../MessageTypes.hpp"

using asio::ip::udp;
using json = nlohmann::json;

class Client {
public:
	const static uint32_t maxMessageLength = 2048;
	static uint8_t connTimeoutSec;
	static uint8_t maxRetries;
	static uint8_t timeoutMs;

	Client(const std::string& address, uint16_t port,asio::io_context& ioC);
	~Client() = default;
	std::string connect();
	bool msgToServer(json message);
	json msgFromServer(unsigned short timeOut = timeoutMs);

private:
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