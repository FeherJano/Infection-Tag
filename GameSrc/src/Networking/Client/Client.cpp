#include "Client.hpp"
#include "../../Utility/logging.hpp"
#include <thread>
#include "asio.hpp"

uint8_t Client::maxRetries = 3;



Client::Client(const std::string& address, uint16_t port, asio::io_context& ioC): port(port), 
    mainSocket(ioC), myKiller(nullptr), mySurvivor(nullptr),recvBuf(std::array<char,maxMessageLength>()),
    sendBuf(std::array<char, maxMessageLength>()),currentState(cStateMenu){
    recvBuf.fill(0);
    sendBuf.fill(0);
    udp::resolver resolver = udp::resolver(ioC);
    remoteSendEndp = *resolver.resolve(udp::v4(), address,std::to_string(port)).begin();
    mainSocket.open(udp::v4());
}

void Client::setState(clientState newS) {
    currentState = newS;
}


std::string Client::connect() {
    try {
        json connMsg;
        connMsg[msgTypes::msgType] = messageSet::connReq;
        int retries = 0;
        while (!msgToServer(connMsg) && retries < maxRetries) {
            retries++;
        }
        json response = msgFromServer();
        if (response.at(msgTypes::msgType) != messageSet::OK) {
            return "";
        }
        playerId = response.at("playerId").template get<std::string>();
        std::cout << playerId<<'\n';
        return playerId;
    }
    catch (json::exception e) {
        logErr(e.what());
    }
    catch (std::exception e) {
        logErr(e.what());
    }
    return "";
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

json Client::msgFromServer() {
    json response;
    try {
        size_t len = mainSocket.receive_from(asio::buffer(recvBuf), remoteRecieveEndp);
        logInfo("Msg from server: " << recvBuf.data());
        response = json::parse(recvBuf.data());
    }
    catch (json::exception e) {
        logErr("Got an invalid json from server: " << e.what());
    }
    catch (std::exception e){
        logErr("Error during recieving from server!"<<e.what());
    }
    return response;

}


void Client::waitForGame() {
    while (currentState == cStateWaitGame) {
        try {
            json msg = msgFromServer();
            if (msg.at(msgTypes::msgType) == messageSet::gameStart) {
                currentState = cStateStartGame;
            }
        }
        catch (json::exception e) {
            logErr(e.what());
        }
        catch (std::exception e) {
            logErr("Fatal during waiting to start, " << e.what());
        }
    }



}