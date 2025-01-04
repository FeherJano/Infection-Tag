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



void Client::waitForGameData() {
    try {
        char buffer[1024];
        asio::ip::udp::endpoint senderEndpoint;

        // Adatok fogadása
        size_t len = socket.receive_from(asio::buffer(buffer), senderEndpoint);
        json gameData = json::parse(std::string(buffer, len));

        std::cout << "Game data received:\n" << gameData.dump(4) << std::endl;

        // Nyugtázó üzenet küldése a szervernek
        json ackMsg;
        ackMsg["type"] = "ack";
        socket.send_to(asio::buffer(ackMsg.dump()), senderEndpoint);

        std::cout << "Acknowledgment sent to server." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error receiving game data: " << e.what() << std::endl;
    }
}
