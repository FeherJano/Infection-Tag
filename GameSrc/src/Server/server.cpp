#include "server.hpp"
#include <iostream>

unsigned short CatGameServer::bindToPort() {
	unsigned short port = this->listenerPort;
	while (clientListener.listen(port) != sf::Socket::Done) {
		port++;
	}
	return port;
}


void CatGameServer::listen() {

	if ( clientListener.accept( client ) != sf::Socket::Done) {
		throw new std::exception("Error occurred during waiting to client connection!");
	}
	std::cout << "Accepted client " << std::endl;
//	std::cout << "Accepted client no." << clients.size()<<'\n';


}


void CatGameServer::ServerFunction() {

	listen();
	while (true)
	{
		sf::Packet response = sf::Packet();
		auto status = client.receive(response);
		std::cout << "Got message from client! " << response.getData();
		if (status != sf::Socket::Done) {
			std::cout << "Err happened while recieving: "<<status << std::endl;
			continue;
		}

		auto message = sf::Packet();
		std::string msg = "Hello client";
		message.append(&msg, sizeof(message));
		client.send(message);
	}

}


