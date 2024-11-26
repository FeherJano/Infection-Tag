#include "Client.hpp"
#include "../../Utility/logging.hpp"
#include <thread>
#include "asio.hpp"

uint8_t Client::connTimeoutSec = 1;
uint8_t Client::timeoutMs = 100;
uint8_t Client::maxRetries = 3;



Client::Client(const std::string& address, uint16_t port, asio::io_context& ioC): port(port),ioContext(ioC), 
    mainSocket(ioContext), myKiller(nullptr), mySurvivor(nullptr) {

    udp::resolver resolver = udp::resolver(ioContext);
    remoteSendEndp = *resolver.resolve(udp::v4(), address,std::to_string(port)).begin();
    mainSocket.open(udp::v4());
    logInfo("Client opened socket on " << mainSocket.local_endpoint().address().to_string());
}


std::string Client::connect() {
    try {
        json connMsg;
        connMsg["message_type"] = messageSet::connReq;
        int retries = 0;
        while (!msgToServer(connMsg) && retries < maxRetries) {
            retries++;
        }
        json response = msgFromServer();
        if (response.at("message_type") != messageSet::OK) {
            return "";
        }
        //TODO save player id
        std::cout << response.at("playerId")<<'\n';
    }
    catch (json::exception e) {

    }
    catch (std::exception e) {

    }
}


bool Client::msgToServer(json message) {

    try {
        mainSocket.send_to(asio::buffer(message.dump()), remoteSendEndp);
    }
    catch (std::exception e) {
        logErr("An exception occured during messaging server!"<<e.what());
        return false;
    }
    return true;

}

json Client::msgFromServer(unsigned short timeout) {
    json response;
    try {
        
        size_t len = mainSocket.receive_from(asio::buffer(recv_buf), remoteRecieveEndp);
        std::cout.write(recv_buf.data(), len);
        response = json::parse(recv_buf.data());
    }
    catch (json::exception e) {
        logErr("Got an invalid json from server: " << e.what());
    }
    catch (std::exception e){
        logErr("Error during recieving from server!"<<e.what());
    }
    return response;

}