#pragma once
#include "SFML/Network.hpp"

class Client {
	sf::TcpSocket Server;
	sf::IpAddress serverAddress;
	uint16_t port;
protected:
    sf::TcpSocket& getServer();

public:
	static sf::Time connTimeoutSec;
	static unsigned short maxRetries;
	static unsigned short timeoutMs;

	Client(const std::string& address, uint16_t port);
	~Client() = default;

    sf::IpAddress getServerAddress() const;
    uint16_t getPort() const;
    static sf::Time getConnTimeoutSec();
	
    void setServerAddress(const sf::IpAddress& address);
    void setPort(uint16_t newPort);
    static void setConnTimeoutSec(unsigned short newTimeout);
	sf::Socket::Status connectToServer(sf::Time timeout = Client::connTimeoutSec);

	bool msgToServer(sf::Packet &msg);
	sf::Packet msgFromServer(unsigned short timeOut = timeoutMs);


};