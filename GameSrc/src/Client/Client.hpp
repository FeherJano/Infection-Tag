#pragma once
#include "nlohmann/json.hpp"
using json = nlohmann::json;
#include "asio.hpp"
//TODO MAKE COMMUNICATION JSON BASED!!!!!

class Client {
	uint16_t port;
protected:
	
public:
	static unsigned short connTimeoutSec;
	static unsigned short maxRetries;
	static unsigned short timeoutMs;

	Client(const std::string& address, uint16_t port);
	~Client() = default;

    uint16_t getPort() const;
    static unsigned short getConnTimeoutSec();
	
    void setPort(uint16_t newPort);
    static void setConnTimeoutSec(unsigned short newTimeout);

	bool msgToServer(std::string message);
	json msgFromServer(unsigned short timeOut = timeoutMs);


};