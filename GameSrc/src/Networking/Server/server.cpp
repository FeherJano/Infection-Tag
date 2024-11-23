#include "Server.hpp"
#include <iostream>
#include <thread>
#include "../../Utility/logging.hpp"
#include  <array>

const unsigned short CatGameServer::defaultPort = 8085;
const unsigned CatGameServer::maximumMessageLength = 2048;


CatGameServer::CatGameServer(asio::io_context& ioC, uint16_t desiredPort = CatGameServer::defaultPort) :
	mainSocket(udp::socket(ioC, udp::endpoint(udp::v4(), desiredPort))), aviableMessages(messageSet()) {
	log("Server started on adress: " << mainSocket.local_endpoint().address().to_string(), logLevelInfo);
}

CatGameServer::~CatGameServer() {
	mainSocket.close();
	messageQueue.clear();
}


serverState CatGameServer::getCurrentState()const {
	return this->currentState;
}

void CatGameServer::setState(serverState nS) {
	currentState = nS;
}

void CatGameServer::setPlayerCount(uint8_t pc) {
	if (pc > maxPlayers)return;
	playerCount = pc;
}

uint8_t CatGameServer::getPlayerCount()const {
	return this->playerCount;
}



bool CatGameServer::broadcastMessage(json msg) {
	try {
		mainSocket.send(asio::buffer(msg.dump()));
	}
	catch (std::exception e) {
		logErr("Failed to broadcast message on server side! Reason: " << e.what());
		return false;
	}
	return true;

}

bool CatGameServer::sendMsg(udp::endpoint to, json msg) {
	try {
		mainSocket.send_to(asio::buffer(msg.dump()), to);
	}
	catch (std::exception e) {
		logErr("Failed to send message to: " << to.address() << " - Reason: " << e.what());
		return false;
	}
	return true;
}


void CatGameServer::listen() {
	std::array<char, 128> recv_buf;
	while (getCurrentState() == serverStateLobby) {
		try {
			udp::endpoint remote_endpoint;
			auto len = mainSocket.receive_from(asio::buffer(recv_buf), remote_endpoint);
			json request = json::parse(recv_buf.data());

			if (request.at("message_type") != aviableMessages.getMsg(messageSet::mt_connReq))continue;


		}
		catch (std::exception e) {
			logErr("Server listening state: " << e.what());
		}
		
		
	}

}



void CatGameServer::ServerFunction() {
	while (1) {
		switch (currentState) {
		case serverStateLobby() :{
			std::thread ListenerThread(&CatGameServer::listen, &(*this));
			ListenerThread.join(); //program waits here until lobby state has changed
			break;
		}
		default:
			break;
		}
	}
	
	

}


		/*this belonged to the serverfunction
		try {

			if (0) {
				std::array<char, 1024> recv_buf;
				udp::endpoint remote_endpoint;
				auto len = mainSocket.receive_from(asio::buffer(recv_buf), remote_endpoint);
				std::cout.write(recv_buf.data(), len);
				std::cout << '\n';
				std::string response = "Hey little fella on: " + remote_endpoint.address().to_string();
				std::error_code ignored_error;
				mainSocket.send_to(asio::buffer(response),
					remote_endpoint, 0, ignored_error);
			}
		}
		catch (std::exception e) {
			logErr(e.what());
		}*/

