#pragma once
#include <list>
#include "SFML/Graphics.hpp"
#include "../Server/server.hpp"
#include "AppState.hpp"
#include "Menu/Button.hpp"


const unsigned short myPort = 8085;

class WindowApplication {
protected:
	const unsigned width, height;
	sf::RenderWindow* mainWindow;

	AppState currentState;
	CatGameServer * gameServer;
	std::list<uiElement*> uiElements;

public:
	WindowApplication(const unsigned width, const unsigned height);
	~WindowApplication();

	void startServer();
	void processInput();
	void renderElements();
	int main();

};