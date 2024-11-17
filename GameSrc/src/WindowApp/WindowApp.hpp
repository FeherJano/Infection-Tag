#pragma once
#include <list>
#include <memory>
#include <vector>
#include "SFML/Graphics.hpp"
#include "../Networking/Server/Server.hpp"
#include "../Networking/Client/Client.hpp"
#include "AppState.hpp"
#include "Menu/Button.hpp"


const unsigned short myPort = 8085;
const std::string localhost = "localhost";

class WindowApp {
protected:
	const unsigned width, height;
	sf::RenderWindow* mainWindow;

	AppState currentState;
	std::unique_ptr<CatGameServer> gameServer;
	std::unique_ptr<Client> player;
	std::list<uiElement*> uiElements;
	void clientEcho();
	asio::io_context &ioContext;

public:
	WindowApp(asio::io_context& ioC,const unsigned width, const unsigned height);
	~WindowApp();

    void startServer();
    void startClient();
    int main();

private:
    void processInput();
    void renderElements();

    void initializeMenu();
    void initializePlayState();
    void initializeLobbyStateHost();
    void initializeLobbyStateClient();
	void initializeJoinState();
};
