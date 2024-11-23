#pragma once
#include "nlohmann/json.hpp"
#include "asio.hpp"

using asio::ip::udp;
using json = nlohmann::json;

class Client {
	uint16_t port;
	asio::io_context &ioContext;
	udp::resolver resolver;
	udp::endpoint recieverPoint;
	udp::endpoint senderPoint;
	udp::socket mainSocket;
	
public:
	static unsigned short connTimeoutSec;
	static unsigned short maxRetries;
	static unsigned short timeoutMs;

	Client(const std::string& address, uint16_t port,asio::io_context& ioC);
	~Client() = default;

    uint16_t getPort() const;	
    void setPort(uint16_t newPort);

	bool msgToServer(std::string message);
	json msgFromServer(unsigned short timeOut = timeoutMs);


};