#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include "../Utility/Exceptions.hpp"

class CatGameServer{
private:
	sf::TcpListener clientListener;
	sf::TcpSocket client;
	unsigned short listenerPort;


	unsigned short bindToPort();
	void listen();

public:
	CatGameServer(unsigned short desiredPort) : listenerPort(desiredPort) {
		listenerPort = this->bindToPort();
	}
	~CatGameServer() {
//		clients.clear();
	}

	void ServerFunction();


};