#include "Client.hpp"
#include "../Utility/logging.hpp"
#include <thread>

unsigned short Client::connTimeoutSec = 1;
unsigned short Client::timeoutMs = 30;
unsigned short Client::maxRetries = 3;


Client::Client(const std::string& address, uint16_t port): port(port) {
	
}



// Getters

uint16_t Client::getPort() const {
    return port;
}


// Setters

void Client::setPort(uint16_t newPort) {
    port = newPort;
}


bool Client::msgToServer(std::string message) {

    short attempts = 0;
    try {
        while (attempts < Client::maxRetries) {
           
            std::this_thread::sleep_for(std::chrono::milliseconds(Client::timeoutMs));
            attempts++;
        }
    }
    catch (std::exception e) {
        logErr("An exception occured during messaging server!"<<e.what());
    }
    return attempts != Client::maxRetries;

}

json Client::msgFromServer(unsigned short timeout) {
    json response = "'{a:5}'"_json;
    try {
       
    }
    catch (std::exception e) {
        logErr("An error occurred during recieving from server!");
    }
    return response;

}