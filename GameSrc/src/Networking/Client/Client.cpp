#include "Client.hpp"
#include <iostream>

Client::Client(const std::string& serverAddress, uint16_t port, asio::io_context& ioContext)
    : socket(ioContext) {
    try {
        // Resolver az IPv4 címekhez
        asio::ip::udp::resolver resolver(ioContext);
        serverEndpoint = *resolver.resolve(asio::ip::udp::v4(), serverAddress, std::to_string(port)).begin();

        // Socket inicializálása IPv4-re
        socket.open(asio::ip::udp::v4());
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing client socket: " << e.what() << std::endl;
    }
}

Client::~Client() {
    player.reset();
    player = nullptr;
    socket.close();
    gameData.clear();
}


std::string Client::connect() {
    try {
        json connMsg;
        connMsg["type"] = "connect";

        // Üzenet küldése a szervernek
        socket.send_to(asio::buffer(connMsg.dump()), serverEndpoint);

        // Válasz fogadása
        char buffer[1024];
        asio::ip::udp::endpoint senderEndpoint;
        size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);

        // Válasz feldolgozása
        json response = json::parse(std::string(buffer, len));

        if (response["type"] == "connected") {
            //std::string playerId = response["playerId"];
            playerId = response["playerId"];
            setId(playerId);
            if (response.contains("role") && !response["role"].is_null()) {
                std::string role = response["role"];

                if (role == "Killer") {
                    // Killer objektum létrehozása
                    player = std::make_unique<Killer>(0, 0, std::array<sf::Keyboard::Key, 4>{sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D});
                }
                else if (role == "Survivor") {
                    // Survivor objektum létrehozása
                    player = std::make_unique<Survivor>(0, 0, std::array<sf::Keyboard::Key, 4>{sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D});
                }
                else {
                    std::cerr << "Unknown role received: " << role << std::endl;
                    return "";
                }

                // A szerver által küldött karakteradatok beállítása
                if (response.contains("playerData")) {
                    player->from_json(response["playerData"]);
                }

                return playerId;
            }
            else {
                std::cerr << "Error: 'role' is missing or null in server response." << std::endl;
                return "";
            }
        }
        else {
            std::cerr << "Unexpected response from server: " << response.dump() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
    }

    std::cerr << "Connect failed." << std::endl;
    return "";
}


std::vector<std::vector<int>> Client::decompressMap(const std::vector<std::vector<std::pair<int, int>>>& compressedMap) {
    std::vector<std::vector<int>> decompressedMap;
    for (const auto& compressedRow : compressedMap) {
        std::vector<int> row;
        for (const auto& [value, count] : compressedRow) {
            row.insert(row.end(), count, value); // Az értékek kibontása
        }
        decompressedMap.push_back(row);
    }
    return decompressedMap;
}


void Client::cListen(std::function<void(const json&)> onDataReceived) {
    try {
        std::string fullData;
        bool initialDataProcessed = false;

        while (true) {
            char buffer[1024];
            asio::ip::udp::endpoint senderEndpoint;
            size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);

            std::string chunk(buffer, len);

            if (!initialDataProcessed) {
                // Inicializációs adatokat fogadunk
                if (chunk == "END_OF_DATA") {
                    try {
                        gameData = json::parse(fullData);

                        // Debug: Ellenőrzés az összeállított adatról
                        std::cout << "Received initial full data: " << fullData.size() << " bytes" << std::endl;

                        // Tömörített térkép visszaállítása
                        auto compressedMap = gameData["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
                        auto decompressedMap = decompressMap(compressedMap);

                        std::cout << "Decompressed map size: " << decompressedMap.size() << "x" << decompressedMap[0].size() << std::endl;

                        // Nyugtázás a szerver felé
                        json ackMsg;
                        ackMsg["type"] = "ack";
                        socket.send_to(asio::buffer(ackMsg.dump()), senderEndpoint);

                        // Jelöljük az inicializációs adatok feldolgozottságát
                        gameDataReady = true;
                        initialDataProcessed = true;
                        fullData.clear();

                        // Továbbítjuk az inicializációs adatokat
                        onDataReceived(gameData);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Error parsing initial JSON data: " << e.what() << std::endl;
                    }
                }
                else {
                    fullData += chunk; // Csomagokat összefűzzük
                }
            }
            else {
                // Játékbeli frissítések fogadása
                try {
                    if (chunk == "EXIT") {
                        json exitCall;
                        exitCall["type"] = "reset";
                        onDataReceived(exitCall);
                        break;
                    }
                    if (chunk == "END_OF_DATA") {
                        if (!fullData.empty()) {
                            json parsedData = json::parse(fullData);

                            // Debug: Ellenőrzés az érkező adatról
                            std::cout << "Received update: " << fullData.size() << " bytes" << std::endl;

                            // Továbbítjuk az adatokat
                            onDataReceived(parsedData);

                            fullData.clear(); // Buffer ürítése a következő üzenethez
                        }
                    }
                    else {
                        fullData += chunk; // Csomag hozzáfűzése
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error parsing update JSON data: " << e.what() << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in cListen: " << e.what() << std::endl;
    }
}


void Client::ClientFunction() {
    while (true) {
        std::cout << "Client listening for data..." << std::endl;
        //listen();

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Minimalis várakozás a CPU túlterhelése elkerülése érdekében
    }
}


void Client::sendPlayerInput(const sf::Vector2f& direction) {
    try {
        json inputMsg;
        inputMsg["type"] = "input";
        inputMsg["direction"] = { direction.x, direction.y };
        socket.send_to(asio::buffer(inputMsg.dump()), serverEndpoint);
    }
    catch (const std::exception& e) {
        std::cerr << "Error sending player input: " << e.what() << std::endl;
    }
}
