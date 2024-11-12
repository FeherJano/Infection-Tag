#pragma once
#include <list>
#include "SFML/Graphics.hpp"
#include "../Server/server.hpp"
#include "../Client/Client.hpp"
#include "AppState.hpp"
#include "Menu/Button.hpp"


const unsigned short myPort = 8088;
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
	void processInput();
	void renderElements();
	int main();

};