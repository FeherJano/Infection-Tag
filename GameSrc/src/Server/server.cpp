#include "server.hpp"
#include <iostream>
#include <thread>
#include "../Utility/logging.hpp"
#include <sstream>

const unsigned short CatGameServer::defaultPort = 8088;


unsigned short CatGameServer::bindToPort() {
	unsigned short port = this->listenerPort;
	while (client.bind(defaultPort) != sf::Socket::Done) {
		port++;
	}
	return port;
}


void CatGameServer::listen() {
	log("Server started listening on port: "<< this->listenerPort, loggingInfo);
	char data[1024];

	std::cout << "Accepted client " << std::endl;

//	std::cout << "Accepted client no." << clients.size()<<'\n';


}


void CatGameServer::ServerFunction() {
	std::thread ListenerThread(&CatGameServer::listen, &(*this));
	ListenerThread.join();
	char data[1024];
	while (true)
	{
		sf::Packet response = sf::Packet();
		//auto status = client.receive(data,1024);
		switch (status) {
		case sf::Socket::Disconnected:
			log("Terminated connection! Closing thread..", loggingInfo);
			return;
			break;
		case sf::Socket::Error:
			log("Error occurred during recieving from client!", loggingErr);
			break;
		default:
			break;
		}
		std::string msg;
		response >> msg;
		std::cout << "Got message from client! " << msg<<'\n';
		
		auto message = sf::Packet();
		msg = "Hello client";
		message << msg;
		client.send(message);
	}

}


