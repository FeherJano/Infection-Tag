#include "Server.hpp"
#include <iostream>
#include <thread>
#include "../../Utility/logging.hpp"
#include  <array>

const uint16_t CatGameServer::defaultPort = 8085;
const uint32_t CatGameServer::maximumMessageLength = 2048;
const uint8_t CatGameServer::maxPlayers = 8;


CatGameServer::CatGameServer(asio::io_context& ioC, uint16_t desiredPort) :
	mainSocket(udp::socket(ioC, udp::endpoint(udp::v4(), desiredPort))),
	players(std::unordered_map<std::string,std::pair<udp::endpoint,std::queue<json>>>()),playerCount(0),currentState(serverStateIdle){
	//log("Server started on adress: " << mainSocket.local_endpoint().address().to_string(), logLevelInfo);
}

CatGameServer::~CatGameServer() {
	mainSocket.close();
	players.clear();
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
		for (auto it : players){
			mainSocket.send_to(asio::buffer(msg.dump()), it.second.first);
		}
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

/*@param playerAddress the address of the player to be registered
* @returns the playerID of the registered player or empty string if registration failed.
* 
*/
std::string CatGameServer::registerPlayer(udp::endpoint playerAddress) {
	if (getPlayerCount() >= maxPlayers) {
		return "";
	}
	std::string id = "Player " + std::to_string(getPlayerCount() + 1);
	players[id] = std::pair<udp::endpoint, std::queue<json>>(playerAddress, std::queue<json>());
	players[id].first= playerAddress;
	setPlayerCount(getPlayerCount() + 1);
	return id;
}

void CatGameServer::listen() {
	std::array<char, 128> recv_buf;
	recv_buf.fill(0);
	while (getCurrentState() == serverStateLobby) {
		try {
			udp::endpoint remote_endpoint;
			auto len = mainSocket.receive_from(asio::buffer(recv_buf), remote_endpoint);
			json request = json::parse(recv_buf.data());

			//skipping everything that is not a connection request
			if (request.at("message_type") != messageSet::connReq)continue;
			auto pId = registerPlayer(remote_endpoint);
			if (pId != "") {
				json response;
				response["message_type"] = messageSet::OK;
				response["playerId"] = pId;
				mainSocket.send_to(asio::buffer(response.dump()), remote_endpoint);
			}

		}
		catch (json::exception e) {
			logErr("Json exception, " << e.what());
		}
		catch (std::exception e) {
			logErr(e.what());
		}
		
		
	}

}


void CatGameServer::shutDown() {
	json abortConnection;
	abortConnection["message_type"] = messageSet::connAbort;
	broadcastMessage(abortConnection);
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

