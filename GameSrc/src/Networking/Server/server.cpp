#include "Server.hpp"
#include <iostream>
#include <thread>
#include "../../Utility/logging.hpp"
#include  <array>

const uint16_t CatGameServer::defaultPort = 8085;
const uint8_t CatGameServer::maxPlayers = 8;


CatGameServer::CatGameServer(asio::io_context& ioC, uint16_t desiredPort) :
	mainSocket(udp::socket(ioC, udp::endpoint(udp::v4(), desiredPort))),
	players(std::unordered_map<std::string,std::pair<udp::endpoint,std::queue<json>>>()),playerCount(0),currentState(serverStateIdle){
	readyPlayers = 1;
	recvBuf = std::array<char, maximumMessageLength>();
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

json CatGameServer::getMsg(udp::endpoint & from) {
	recvBuf.fill(0);
	json msg;
	try {
		auto len = mainSocket.receive_from(asio::buffer(recvBuf), from);
		msg = json::parse(recvBuf.data());
		return msg;
	}
	catch (json::exception e) {
		logErr(e.what());
	}catch (std::exception e) {
		logErr(e.what());
	}
	return msg;
}

/*@param playerAddress the address of the player to be registered
* @returns the playerID of the registered player or empty string if registration failed.
* 
*/
std::string CatGameServer::registerPlayer(udp::endpoint playerAddress) {
	if (getPlayerCount() >= maxPlayers-1) { //duh because one player will be the host 
		return "";
	}
	std::string id = "Player " + std::to_string(getPlayerCount() + 1);
	players[id] = std::pair<udp::endpoint, std::queue<json>>(playerAddress, std::queue<json>());
	players[id].first= playerAddress;
	setPlayerCount(getPlayerCount() + 1);
	return id;
}

void CatGameServer::listen() {
	while (getCurrentState() == serverStateLobby) {
		try {
			udp::endpoint remote_endpoint;
			json request = getMsg(remote_endpoint);

			//skipping everything that is not a connection request
			if (request.at(msgTypes::msgType) != messageSet::connReq)continue;

			auto pId = registerPlayer(remote_endpoint);
			//if registration was successfull, the server sends the players id to the player
			if (pId != "") {
				json response;
				response[msgTypes::msgType] = messageSet::OK;
				response["playerId"] = pId;
				mainSocket.send_to(asio::buffer(response.dump()), remote_endpoint);
				std::thread t(&CatGameServer::handlePlayer, &(*this), pId);
				t.detach();
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
	abortConnection[msgTypes::msgType] = messageSet::connAbort;
	broadcastMessage(abortConnection);
	setState(serverStateExit);
}

bool CatGameServer::startGame() {
	json startSignal;
	startSignal[msgTypes::msgType] = messageSet::gameStart;
	broadcastMessage(startSignal);

	return true;
}


void CatGameServer::ServerFunction() {
	while (currentState!=serverStateExit) {
		switch (currentState) {
		case serverStateLobby: {
			std::thread ListenerThread(&CatGameServer::listen, &(*this));
			ListenerThread.join(); //program waits here until lobby state has changed
			break;
		}
		case serverStateGameStart: {
			//TODO implement start logic
			break;
		}

		default:
			break;
		}
	}

}

void CatGameServer::handlePlayer(std::string playerID) {
	try {
		while (currentState != serverStateExit) {
			json prevMsg = getMsg(players[playerID].first);
			if (prevMsg.at(msgTypes::msgType) == messageSet::clientReady) {
				prevMsg.at(msgTypes::playerData) == true ? playerCount++ : playerCount--;
				logInfo(playerID + " sent ready");
			}
		}
	}
	catch (json::exception e) {
		logErr(e.what());
	}
	catch (std::exception e) {
		logErr(e.what());
	}
}

