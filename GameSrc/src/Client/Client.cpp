#include "Client.hpp"
#include "../Utility/logging.hpp"
#include <thread>

sf::Time Client::connTimeoutSec = sf::seconds(20);
unsigned short Client::timeoutMs = 30;
unsigned short Client::maxRetries = 3;


Client::Client(const std::string& address, uint16_t port): port(port) {
	connTimeoutSec = sf::seconds(20);
	serverAddress = sf::IpAddress(address.c_str());
}

sf::Socket::Status Client::connectToServer(sf::Time timeout ) 
{
	return Server.connect(this->serverAddress, this->port);
}


// Getters
sf::IpAddress Client::getServerAddress() const {
    return serverAddress;
}

uint16_t Client::getPort() const {
    return port;
}

sf::TcpSocket& Client::getServer() {
    return Server;
}

sf::Time Client::getConnTimeoutSec() {
    return connTimeoutSec;
}

// Setters

void Client::setServerAddress(const sf::IpAddress& address) {
    serverAddress = address;
}

void Client::setPort(uint16_t newPort) {
    port = newPort;
}

void Client::setConnTimeoutSec(unsigned short newTimeout) {
    connTimeoutSec = sf::seconds(newTimeout);
}

bool Client::msgToServer(sf::Packet &msg) {

    short attempts = 0;
    try {
        while (attempts < Client::maxRetries) {
            if (getServer().send(msg) != sf::Socket::Partial) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(Client::timeoutMs));
            attempts++;
        }
    }
    catch (std::exception e) {
        logErr("An exception occured during messaging server!"<<e.what());
    }
    return attempts != Client::maxRetries;

}

sf::Packet Client::msgFromServer(unsigned short timeout) {
    auto response = sf::Packet();
    try {
        this->getServer().receive(response);
    }
    catch (std::exception e) {
        logErr("An error occurred during recieving from server!");
    }
    return response;

}