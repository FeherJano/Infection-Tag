#include "Client.hpp"
#include "../Utility/logging.hpp"
#include <thread>
#include "asio.hpp"


unsigned short Client::connTimeoutSec = 1;
unsigned short Client::timeoutMs = 30;
unsigned short Client::maxRetries = 3;


Client::Client(const std::string& address, uint16_t port): port(port), resolver(udp::resolver(ioContext)),mainSocket(ioContext) {
    recieverPoint = *resolver.resolve(udp::v4(), address, "Server").begin();
    mainSocket.open(udp::v4());
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

    try {
        mainSocket.send_to(asio::buffer(message), recieverPoint);
    }
    catch (std::exception e) {
        logErr("An exception occured during messaging server!"<<e.what());
        return false;
    }
    return true;

}

json Client::msgFromServer(unsigned short timeout) {
    json response = "'{a:5}'"_json;
    try {
        std::array<char, 128> recv_buf;
        udp::endpoint sender_endpoint;
        size_t len = mainSocket.receive_from(
            asio::buffer(recv_buf), sender_endpoint);
            std::cout.write(recv_buf.data(), len);
    }
    catch (std::exception e) {
        logErr("An error occurred during recieving from server!");
    }
    return response;

}