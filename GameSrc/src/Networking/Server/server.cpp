#include "Server.hpp"

CatGameServer::CatGameServer(asio::io_context& ioContext, uint16_t port)
    : socket(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), currentState(serverStateLobby) {
    std::cout << "Server initialized in lobby state." << std::endl;
}


void CatGameServer::setState(serverState newState) {
    currentState = newState;
}

void CatGameServer::ServerFunction() {
    while (true) {
        if (currentState == serverStateLobby) {
            std::cout << "Server is in lobby state. Listening for connections..." << std::endl;
            listen();
        }
        if (currentState == serverStateGameStart) {
            std::cout << "Server is starting the game. Setting up game state..." << std::endl;
            setupGameState();
            currentState = serverStateGame; // Állapot frissítése a játék indítása után
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Minimalis várakozás a CPU túlterhelése elkerülése érdekében
    }
}


void CatGameServer::listen() {
    try {
        char buffer[1024];
        asio::ip::udp::endpoint senderEndpoint;

        while (currentState == serverStateLobby || currentState == serverStateGame) {
            size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);

            json request = json::parse(std::string(buffer, len));
            if (request["type"] == "connect") {
                static int playerCounter = 1;
                std::string playerId = "Player" + std::to_string(playerCounter++);

                json response;
                response["type"] = "connected";
                response["playerId"] = playerId;

                socket.send_to(asio::buffer(response.dump()), senderEndpoint);

                playersEndpoints[playerId] = senderEndpoint;
                std::cout << "Player connected: " << playerId << " at "
                    << senderEndpoint.address().to_string() << ":" << senderEndpoint.port() << std::endl;
            }
            else if (request["type"] == "ack") {
                std::cout << "Acknowledgment received from "
                    << senderEndpoint.address().to_string() << ":" << senderEndpoint.port() << std::endl;
            }
            else {
                std::cerr << "Unknown request type: " << request.dump() << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in server listen: " << e.what() << std::endl;
    }
}


void CatGameServer::setupGameState() {
    // Generáljuk az adatokat
    gameData["map"] = { {0, 0, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0} }; // Példa térkép
    gameData["tasks"] = { {"position", {2, 2}}, {"progress", 0} }; // Példa feladatok
    gameData["players"] = { {"playerId", "Player1"}, {"position", {3, 0}} }; // Példa játékos

    std::cout << "Game state setup completed. Broadcasting data..." << std::endl;

    // Meghívjuk a broadcastGameData-t
    broadcastGameData();
}



void CatGameServer::generateGameData() {
    // Generáljuk a térképet
    std::vector<std::vector<int>> map(10, std::vector<int>(10, 0));
    for (int i = 0; i < 10; ++i) {
        map[i][i] = 1; // Átlós fal
    }

    // Feladatok
    std::vector<json> tasks;
    tasks.push_back({ {"position", {2, 2}}, {"progress", 0} });
    tasks.push_back({ {"position", {5, 5}}, {"progress", 0} });

    // Játékosok
    std::vector<json> players;
    for (const auto& [playerId, endpoint] : playersEndpoints) {
        players.push_back({ {"playerId", playerId}, {"position", {0, 0}} });
    }

    // JSON objektum készítése
    gameData = {
        {"map", map},
        {"tasks", tasks},
        {"players", players}
    };

    std::cout << "Game data generated:\n" << gameData.dump(4) << std::endl;
}



void CatGameServer::broadcastGameData() {
    for (const auto& [playerId, endpoint] : playersEndpoints) {
        try {
            std::string dataToSend = gameData.dump();
            auto buffer = asio::buffer(dataToSend);

            // Adatok elküldése
            socket.send_to(buffer, endpoint);
            std::cout << "Game data sent to player " << playerId << " at "
                << endpoint.address().to_string() << ":" << endpoint.port() << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to send game data to player " << playerId << ": " << e.what() << std::endl;
        }
    }
}





