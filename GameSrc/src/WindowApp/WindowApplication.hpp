#pragma once
#include <list>
#include "SFML/Graphics.hpp"
#include "../Server/server.hpp"
#include "../Client/Client.hpp"
#include "AppState.hpp"
#include "Menu/Button.hpp"


const unsigned short myPort = 8088;

class WindowApplication {
protected:
	const unsigned width, height;
	sf::RenderWindow* mainWindow;

	AppState currentState;
	std::unique_ptr<CatGameServer> gameServer;
	std::unique_ptr<Client> player;
	std::list<uiElement*> uiElements;

public:
	WindowApplication(const unsigned width, const unsigned height);
	~WindowApplication();

	void startServer();
	void processInput();
	void renderElements();
	int main();

};