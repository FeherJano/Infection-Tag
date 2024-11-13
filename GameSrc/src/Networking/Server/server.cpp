#include "server.hpp"
#include <iostream>
#include <thread>
#include "../../Utility/logging.hpp"
#include  <array>

const unsigned short CatGameServer::defaultPort = 8085;


serverState CatGameServer::getCurrentState()const {
	return this->currentState;
}

void CatGameServer::changeState(serverState nS) {
	currentState = nS;
}


void CatGameServer::listen() {
	std::array<char, 128> recv_buf;
	while (getCurrentState() == stateLobby) {
		udp::endpoint remote_endpoint;
		auto len = mainSocket.receive_from(asio::buffer(recv_buf), remote_endpoint);
		
	}

}


void CatGameServer::ServerFunction() {
	if (0) {
		std::thread ListenerThread(&CatGameServer::listen, &(*this));
		ListenerThread.join();
	}
	while (1) {
		try {
				std::array<char, 1024> recv_buf;
				udp::endpoint remote_endpoint;
				auto len = mainSocket.receive_from(asio::buffer(recv_buf), remote_endpoint);
				std::cout.write(recv_buf.data(), len);
				std::cout.flush();
				std::string response = "Hey little fella on: " + remote_endpoint.address().to_string();

				std::error_code ignored_error;
				mainSocket.send_to(asio::buffer(response),
					remote_endpoint, 0, ignored_error);
			}
			catch (std::exception e) {
				logErr(e.what());
			}
	}
	
	

}


