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



std::string Client::connect() {
    try {
        json connMsg;
        connMsg["type"] = "connect";

        // Log: üzenet küldése
        std::cout << "Sending connect request to server at "
            << serverEndpoint.address().to_string() << ":"
            << serverEndpoint.port() << std::endl;

        // Üzenet küldése
        socket.send_to(asio::buffer(connMsg.dump()), serverEndpoint);

        // Válasz fogadása
        char buffer[1024];
        asio::ip::udp::endpoint senderEndpoint;
        size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);

        // Válasz feldolgozása
        json response = json::parse(std::string(buffer, len));
        std::cout << "Received response: " << response.dump() << std::endl;

        if (response["type"] == "connected") {
            std::cout << "Connected to server, received playerId: " << response["playerId"] << std::endl;
            return response["playerId"];
        }
        else {
            std::cerr << "Unexpected response from server: " << response.dump() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
    }

    std::cerr << "Connect failed." << std::endl;
    return ""; // Üres string visszaadása, ha a kapcsolat nem sikerült
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


void Client::waitForGameData() {
    try {
        char buffer[1024];
        asio::ip::udp::endpoint senderEndpoint;
        std::string fullData;

        while (true) {
            size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);
            std::string chunk(buffer, len);

            // Ellenőrizd az "END_OF_DATA" jelzést
            if (chunk == "END_OF_DATA") {
                break;
            }

            fullData += chunk; // Folyamatosan építsd a teljes adatot
        }

        // Debug: Ellenőrizd az összeállított adatot
        std::cout << "Received full data: " << fullData << std::endl;

        // JSON parszolás
        gameData = json::parse(fullData);
        //std::cout << "Parsed game data successfully.\n" << gameData.dump(4) << std::endl;

        // Tömörített map visszaállítása
        auto compressedMap = gameData["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
        auto decompressedMap = decompressMap(compressedMap);

        std::cout << "Decompressed map size: " << decompressedMap.size() << "x" << decompressedMap[0].size() << std::endl;

        // Nyugtázó üzenet küldése a szervernek
        json ackMsg;
        ackMsg["type"] = "ack";
        socket.send_to(asio::buffer(ackMsg.dump()), senderEndpoint);

        std::cout << "Acknowledgment sent to server. Game data ready." << std::endl;

        // Jelöljük az adatok állapotát
        gameDataReady = true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error receiving game data: " << e.what() << std::endl;
    }
}

