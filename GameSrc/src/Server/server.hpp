#pragma once
#include <vector>
#include "nlohmann/json.hpp"
#include "asio.hpp"

using nlohmann::json;
using asio::ip::udp;

//TODO MAKE COMMUNICATION JSON BASED!!!!!
class CatGameServer{
private:
	unsigned short listenerPort;

	unsigned short bindToPort();
	void listen();

	asio::io_context ioContext;
	udp::socket mainSocket;


public:
	CatGameServer(unsigned short desiredPort = CatGameServer::defaultPort) : listenerPort(desiredPort),mainSocket(ioContext, (udp::v4(), defaultPort)){
		listenerPort = this->bindToPort();
	}
	~CatGameServer() {
//		clients.clear();
	}

	void ServerFunction();

	const static unsigned short defaultPort;

};