#pragma once

#include "SFML/Graphics.hpp"
#include "../Server/server.hpp"
#include "AppState.hpp"
const unsigned short myPort = 8085;

class WindowApplication {
protected:
	const unsigned width, height;
	sf::RenderWindow* mainWindow;
	AppState currentState;
	CatGameServer * gameServer;

public:
	WindowApplication(const unsigned width = 1024, const unsigned height = 768) : width(width), height(height), gameServer(nullptr){
		mainWindow = new sf::RenderWindow(sf::VideoMode(width, height), "Freaking cats");
		currentState = appInit;
	}
	~WindowApplication() {
		mainWindow->close();
		delete mainWindow;
		delete gameServer;
	}

	void processInput();
	int main();

};