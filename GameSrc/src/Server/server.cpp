#include "server.hpp"
#include <iostream>
#include <thread>
#include "../Utility/logging.hpp"
#include <sstream>

const unsigned short CatGameServer::defaultPort = 8088;


unsigned short CatGameServer::bindToPort() {
	unsigned short port = this->listenerPort;

	return port;
}


void CatGameServer::listen() {
	log("Server started listening on port: "<< this->listenerPort, loggingInfo);

//	std::cout << "Accepted client no." << clients.size()<<'\n';


}


void CatGameServer::ServerFunction() {
	std::thread ListenerThread(&CatGameServer::listen, &(*this));
	ListenerThread.join();


}


