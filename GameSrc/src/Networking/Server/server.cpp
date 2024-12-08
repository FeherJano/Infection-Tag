#include "Server.hpp"
#include <iostream>
#include <thread>
#include "../../Utility/logging.hpp"
#include  <array>
#include <vector>
#include <utility>


const uint16_t CatGameServer::defaultPort = 8085;
const uint8_t CatGameServer::maxPlayers = 8;


CatGameServer::CatGameServer(asio::io_context& ioC, uint16_t desiredPort) :
	mainSocket(udp::socket(ioC, udp::endpoint(udp::v4(), desiredPort))),
	players(std::unordered_map<std::string,std::pair<udp::endpoint,std::queue<json>>>()),playerCount(1),currentState(serverStateIdle){
	readyPlayers = 1;
	recvBuf = std::array<char, maximumMessageLength>();
	mainSocket.set_option(asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{1000});
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
	}
	catch (asio::system_error e) {
		return msg;
	}
	catch (std::exception e) {
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
			if (request.empty())continue;

			//skipping everything that is not a connection request and sending it to the sender player's message queue
			if (request.at(msgTypes::msgType) != messageSet::connReq) {
				players[request.at(msgTypes::playerId)].second.push(request);
				continue;
			}

			auto pId = registerPlayer(remote_endpoint);
			//if registration was successfull, the server sends the players id to the player
			if (pId != "") {
				json response;
				response[msgTypes::msgType] = messageSet::OK;
				response["playerId"] = pId;
				sendMsg(remote_endpoint, response);
				std::thread t(&CatGameServer::handlePlayer, &(*this), pId);
				t.detach();
			}

		}
		catch (json::exception e) {
			logErr("Json exception, " << e.what());
		}
		catch (std::exception e) {
			logErr("Server lobby state error: " << e.what());
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
			startGame();
			break;
		}

		default:
			break;
		}
	}

}

void CatGameServer::handlePlayer(std::string playerID) {
	auto msgQueue = &players[playerID].second;
	while (currentState != serverStateExit) {
		try {
			if (msgQueue->empty()) {
				std::this_thread::sleep_for(100ms);
				continue;
			}
			json prevMsg = msgQueue->front();
			if (prevMsg.at(msgTypes::msgType) == messageSet::clientReady) {
				prevMsg.at(msgTypes::playerData) == true ? readyPlayers++ : readyPlayers--;
				logInfo(playerID + " sent ready");
			}
			if (prevMsg.at(msgTypes::msgType) == messageSet::connAbort) {
				msgQueue->pop();
				players.erase(playerID);
				setPlayerCount(getPlayerCount() - 1);
				logInfo(playerID + " disconnected");
				readyPlayers--;
				break;
			}
			msgQueue->pop();

		}
		catch (json::exception e) {
			logErr(e.what());
		}
		catch (std::exception e) {
			logErr(e.what());
		}
	}
	
}

// Tömörítés megvalósítása
std::vector<std::vector<std::pair<int, int>>> CatGameServer::compressMap(const std::vector<std::vector<int>>& map) {
	std::vector<std::vector<std::pair<int, int>>> compressedMap;
	for (const auto& row : map) {
		std::vector<std::pair<int, int>> compressedRow;
		int currentValue = row[0];
		int count = 0;

		for (const auto& cell : row) {
			if (cell == currentValue) {
				++count;
			}
			else {
				compressedRow.emplace_back(currentValue, count);
				currentValue = cell;
				count = 1;
			}
		}
		compressedRow.emplace_back(currentValue, count); // Az utolsó szakasz hozzáadása
		compressedMap.push_back(compressedRow);
	}
	return compressedMap;
}


void CatGameServer::setupGameState() {
	std::vector<std::vector<int>> maze(HEIGHT, std::vector<int>(WIDTH, 0));
	placeObjects(maze);

	// Helyezi el a feladatokat
	std::vector<Task> tasks;
	placeTasks(tasks, maze, playerCount - 1);

	// Játékosok elhelyezése
	std::vector<Survivor> survivors;
	Killer killer(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D });
	for (uint8_t i = 0; i < playerCount - 1; ++i) {
		survivors.emplace_back(Survivor(rand() % WIDTH, rand() % HEIGHT, { sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right }));
	}

	// JSON generálása
	json gameState;

	gameState["msg_type"] = messageSet::gameState;

	// Pálya hozzáadása
	std::vector<std::vector<std::pair<int, int>>> compressedMap = compressMap(maze);
	gameState["map"] = compressedMap;

	// Feladatok JSON-reprezentáció
	json tasksJson = json::array();
	for (const auto& task : tasks) {
		tasksJson.push_back(task.to_json());
	}
	gameState["tasks"] = tasksJson;

	// Túlélők JSON-reprezentáció
	json survivorsJson = json::array();
	for (const auto& survivor : survivors) {
		survivorsJson.push_back(survivor.to_json());
	}
	gameState["players"] = survivorsJson;

	// Gyilkos JSON-reprezentáció
	gameState["killer"] = killer.to_json(); // Meghívjuk az objektum saját metódusát


	// Játékosoknak kiküldése
	broadcastMessage(gameState);
}

