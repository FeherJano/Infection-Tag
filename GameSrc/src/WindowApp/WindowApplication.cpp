#include "WindowApplication.hpp"
#include <iostream>
#include <thread>


WindowApplication::WindowApplication(const unsigned width = 1024, const unsigned height = 768) : width(width), height(height), gameServer(nullptr), player(nullptr) {
	mainWindow = new sf::RenderWindow(sf::VideoMode(width, height), "Freaking cats");
	currentState = appInit;

	//TODO Make these magic numbers consistent and make the ui generation cleaner
	uiElement* menuButton1 = new Button(*mainWindow,sf::Vector2f(width/2 - 100, 100), sf::Vector2f(125, 50), "Play", 1U);
	uiElement* menuButton2 = new Button(*mainWindow,sf::Vector2f(menuButton1->getPosition().x, menuButton1->getPosition().y + 100),
		menuButton1->getSize(), "Host", menuButton1->getId()+1);
	uiElements.push_back(menuButton1);
	uiElements.push_back(menuButton2);
}


WindowApplication::~WindowApplication() {
	mainWindow->close();
	delete mainWindow;
	uiElements.clear();
}


void WindowApplication::startServer() {
	if (this->gameServer != nullptr)return;
	this->gameServer = std::unique_ptr<CatGameServer>(new CatGameServer(myPort));
	std::thread serverThread(&CatGameServer::ServerFunction, &(*gameServer));
	serverThread.detach();
} 


void WindowApplication::processInput() {
	sf::Event ev;

	while (mainWindow->pollEvent(ev)) {
		//Window is closed via mouse or escape is pressed
		if (ev.type == sf::Event::Closed || 
			(ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape)) 
		{
			this->mainWindow->close();
		}
		//TODO refactor this shit
		for (auto i : uiElements) {
			if (i->elementFunction(ev)) {
				switch (i->getId())
				{
				case 1:
					return;
				case 2:
					startServer();
					break;
				default:
					break;
				}
			}
		}
	}

	
}


void WindowApplication::renderElements() {

	if (gameServer != nullptr)
		return;
	for (uiElement *i : uiElements) {
		i->renderElement(mainWindow);
	}

}


int WindowApplication::main() {

	while (mainWindow->isOpen()) {
		this->processInput();
		mainWindow->clear(sf::Color::Black);

		renderElements();
		mainWindow->display();
	}
	return 0;
}


