#include "Server.hpp"

CatGameServer::CatGameServer(asio::io_context& ioContext, uint16_t port)
    : socket(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), currentState(serverStateLobby) {
    std::cout << "Server initialized in lobby state." << std::endl;
    playerCounter = 1;
}

CatGameServer::~CatGameServer() {
    currentState = serverStateIdle;
    int retries = 0;
    while (!this->reset() && retries < 3) {
        retries++;
    }
    std::this_thread::sleep_for(20ms);
    survivors.clear();
    maze.clear();
    tasks.clear();
    players.clear();
    playersEndpoints.clear(); //just to make sure
    socket.close();
    playerCounter = 1;

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
        if (currentState == serverStateIdle) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Minimalis várakozás a CPU túlterhelése elkerülése érdekében
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
                
                std::string playerId = "Player" + std::to_string(playerCounter);

                // Csak az első játékos (Player1) lehet Killer
                if (playerCounter == 1 && playerRoles.find(playerId) == playerRoles.end()) {
                    playerRoles[playerId] = "Killer";
                }
                else {
                    playerRoles[playerId] = "Survivor";
                }

                // Következő játékos előkészítése
                playerCounter++;

                json response;
                response["type"] = "connected";
                response["playerId"] = playerId;
                response["role"] = playerRoles[playerId];

                socket.send_to(asio::buffer(response.dump()), senderEndpoint);

                playersEndpoints[playerId] = senderEndpoint;

                std::cout << "Player connected: " << playerId << " at "
                    << senderEndpoint.address().to_string() << ":" << senderEndpoint.port()
                    << " as " << playerRoles[playerId] << std::endl;
            }
            else if (request["type"] == "input") {
                std::string playerId = getPlayerIdByEndpoint(senderEndpoint);
                sf::Vector2f direction(request["direction"][0], request["direction"][1]);
                processPlayerInput(playerId, direction);

                gameData["killer"] = killer.to_json();
                gameData["killer"]["playerId"] = playerId;

                broadcastGameData();
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

void CatGameServer::generateGameData() {
    // Térkép generálása
    maze.resize(HEIGHT, std::vector<int>(WIDTH, 0));
    placeObjects(maze);
    auto compressedMap = compressMap(maze);
    gameData["map"] = compressedMap;

    // Feladatok létrehozása
    tasks.clear();
    placeTasks(tasks, maze, playersEndpoints.size());
    gameData["tasks"] = json::array();
    for (const auto& task : tasks) {
        gameData["tasks"].push_back(task.to_json());
    }

    // Játékosok létrehozása
    survivors.clear();

    auto playerPositions = std::vector<sf::Vector2f>(); //contains the randomized player positions, used to give players nice spawn positions

    // Gyilkos pozíciójának randomizálása
    sf::Vector2f killerPos = generateRandomPosition(maze, 1, 1,playerPositions);
    playerPositions.push_back(killerPos);

    killer.position = killerPos;
    auto killerData = killer.to_json();
    killerData["playerId"] = ""; // Gyilkos azonosítója (alapértelmezés)
    for (const auto& [playerId, role] : playerRoles) {
        if (role == "Killer") {
            killerData["playerId"] = playerId; // Gyilkoshoz rendeljük az azonosítót
            killer = Killer(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D }, playerId);
            break;
        }
    }
    
    gameData["killer"] = killerData;

    // Túlélők pozíciójának randomizálása
    for (const auto& [playerId, role] : playerRoles) {
        if (role == "Survivor") {
            sf::Vector2f survivorPos = generateRandomPosition(maze, 1, 1, playerPositions);
            playerPositions.push_back(survivorPos);
            Survivor survivor(survivorPos.x, survivorPos.y, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D }, playerId);
            survivors.push_back(survivor);

            auto survivorData = survivor.to_json();
            survivorData["playerId"] = playerId; // Survivor-hez azonosító hozzáadása
            gameData["players"].push_back(survivorData);
        }
    }

    std::cout << "Game data generated:\n" << gameData.dump(4) << std::endl; // Debug: Ellenőrizzük a JSON szerkezetet
}



void CatGameServer::broadcastGameData() {
    for (const auto& [playerId, endpoint] : playersEndpoints) {
        try {
            std::string dataToSend = gameData.dump();
            const size_t chunkSize = 1024; // Max buffer size
            size_t totalSize = dataToSend.size();
            size_t numChunks = (totalSize + chunkSize - 1) / chunkSize;

            for (size_t i = 0; i < numChunks; ++i) {
                size_t start = i * chunkSize;
                size_t end = std::min(start + chunkSize, totalSize);
                std::string chunk = dataToSend.substr(start, end - start);

                socket.send_to(asio::buffer(chunk), endpoint);

                // Log the sent chunk
                std::cout << "Sent chunk " << (i + 1) << "/" << numChunks << " to " << playerId << std::endl;
            }

            // Send end of data signal
            std::string endSignal = "END_OF_DATA";
            socket.send_to(asio::buffer(endSignal), endpoint);

        }
        catch (const std::exception& e) {
            std::cerr << "Failed to send game data to player " << playerId << ": " << e.what() << std::endl;
        }

    }
}

void CatGameServer::processPlayerInput(const std::string& playerId, const sf::Vector2f& direction) {
    const float deltaTime = 0.016f; // Példa időköz (60 FPS esetén ~16 ms)

    std::cout << "Processing input for playerId: " << playerId << std::endl;

    if (playerRoles[playerId] == "Survivor") {
        std::cout << "Player is a Survivor." << std::endl;
        // Survivors között keresés
        for (auto& survivor : survivors) {
            auto survivorData = survivor.to_json();
            std::cout << "Checking survivor with playerId: " << survivorData["playerId"] << std::endl;

            if (survivorData["playerId"] == playerId) {
                // Mozgatás az irányvektor alapján
                survivor.moveWithDirection(direction, deltaTime, maze);

                // Frissítsük a gameData-ban a survivor pozícióját
                for (auto& playerData : gameData["players"]) {
                    if (playerData["playerId"] == playerId) {
                        playerData["position"] = { survivor.getPosition().x, survivor.getPosition().y };
                        std::cout << "Updated Survivor position: " << survivor.getPosition().x << ", " << survivor.getPosition().y << std::endl;
                        break;
                    }
                }
                break;
            }
        }
    }
    else if (playerRoles[playerId] == "Killer") {
        std::cout << "Player is a Killer." << std::endl;
        // Killer mozgatása
        killer.moveWithDirection(direction, deltaTime, maze);

        // Frissítsük a gameData-ban a killer pozícióját
        gameData["killer"]["position"] = { killer.getPosition().x, killer.getPosition().y };

        std::cerr << "Killer position: " << killer.getPosition().x << ", " << killer.getPosition().y << std::endl;
    }
    else {
        std::cout << "Unknown role for playerId: " << playerId << std::endl;
    }
}


std::string CatGameServer::getPlayerIdByEndpoint(const asio::ip::udp::endpoint& endpoint) {
    for (const auto& [playerId, playerEndpoint] : playersEndpoints) {
        if (playerEndpoint == endpoint) {
            return playerId;
        }
    }
    throw std::runtime_error("Player not found for the given endpoint.");
}

bool CatGameServer::reset() {
    std::string endOfMessage = "EXIT";
    for (const auto& [playerId, endpoint] : playersEndpoints) {
        socket.send_to(asio::buffer(endOfMessage), endpoint);        
        std::this_thread::sleep_for(1ms);
    }
    return true;

}