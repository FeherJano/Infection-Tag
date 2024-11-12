#pragma once
#include <vector>
#include "nlohmann/json.hpp"
#include "asio.hpp"

using nlohmann::json;
using asio::ip::udp;

//TODO MAKE COMMUNICATION JSON BASED!!!!!
class CatGameServer{
private:

	void listen();
	udp::socket mainSocket;


public:
	CatGameServer(asio::io_context &ioC,unsigned short desiredPort = CatGameServer::defaultPort) : mainSocket(udp::socket(ioC, (udp::v4(), desiredPort))){
	}
	~CatGameServer() = default;
	void start();
	void ServerFunction();

	const static unsigned short defaultPort;

};