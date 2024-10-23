#pragma once
#include <SFML/Network.hpp>
#include <vector>

class CatGameServer{
private:
	sf::TcpListener clientListener;
	sf::TcpSocket client;
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