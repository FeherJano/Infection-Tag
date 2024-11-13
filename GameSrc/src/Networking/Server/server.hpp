#pragma once
#include <vector>
#include "nlohmann/json.hpp"
#include "asio.hpp"
#include "../../Utility/logging.hpp"
using nlohmann::json;
using asio::ip::udp;

enum serverState{stateLobby,stateGameRun,stateIdle};

                
class CatGameServer{
public:
	const static unsigned short defaultPort;

	CatGameServer(asio::io_context& ioC, unsigned short desiredPort = CatGameServer::defaultPort) : mainSocket(udp::socket(ioC, udp::endpoint(udp::v4(), desiredPort))) {
		log("Server started on adress: " << mainSocket.local_endpoint().address().to_string(), logLevelInfo);
	}
	~CatGameServer() {
		mainSocket.close();
	}
	void ServerFunction();
	void changeState(serverState newState);
	serverState getCurrentState()const;

private:
	serverState currentState;
	udp::socket mainSocket;

	void listen();
	bool handShake();


};