#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

//TODO MAKE COMMUNICATION JSON BASED!!!!!
class CatGameServer{
private:
	sf::UdpSocket client;
	unsigned short listenerPort;

	unsigned short bindToPort();
	void listen();

public:
	CatGameServer(unsigned short desiredPort = CatGameServer::defaultPort) : listenerPort(desiredPort) {
		listenerPort = this->bindToPort();
	}
	~CatGameServer() {
//		clients.clear();
	}

	void ServerFunction();

	const static unsigned short defaultPort;

};