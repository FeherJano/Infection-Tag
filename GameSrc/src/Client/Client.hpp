#pragma once
#include "SFML/Network.hpp"

class Client {
	sf::TcpSocket Server;
	sf::IpAddress serverAdress;
public:
	static sf::Time timeoutSec;

	Client(const std::string& adress, uint16_t port) {
		timeoutSec = sf::seconds(20);
		serverAdress = sf::IpAddress(adress.c_str());
		Server.connect(serverAdress,port,timeoutSec);
	}


};