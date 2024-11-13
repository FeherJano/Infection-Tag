#include "WindowApp.hpp"
#include <iostream>
#include <thread>


WindowApp::WindowApp(asio::io_context& ioC, const unsigned width = 1024, const unsigned height = 768) : width(width), height(height),ioContext(ioC), gameServer(nullptr), player(nullptr) {
	mainWindow = new sf::RenderWindow(sf::VideoMode(width, height), "Freaking cats");
	currentState = appInit;

	//TODO Make these magic numbers consistent and make the ui generation cleaner
	uiElement* menuButton1 = new Button(*mainWindow,sf::Vector2f(width/2 - 100, 100), sf::Vector2f(125, 50), "Play", 1U);
	uiElement* menuButton2 = new Button(*mainWindow,sf::Vector2f(menuButton1->getPosition().x, menuButton1->getPosition().y + 100),
		menuButton1->getSize(), "Host", menuButton1->getId()+1);
	uiElements.push_back(menuButton1);
	uiElements.push_back(menuButton2);
}


WindowApp::~WindowApp() {
	mainWindow->close();
	delete mainWindow;
	uiElements.clear();
}


void WindowApp::startServer() {
	if (this->gameServer != nullptr)return;
	this->gameServer = std::unique_ptr<CatGameServer>(new CatGameServer(ioContext,myPort));
	std::thread serverThread(&CatGameServer::ServerFunction, &(*gameServer));
	serverThread.detach();
} 

void WindowApp::clientEcho() {
	while (1) {
		player->msgToServer("Hello man");
		player->msgFromServer();
	}
}

void WindowApp::startClient() {
	if (this->player != nullptr)return;
	player = std::unique_ptr<Client>(new Client("localhost", 8085U, ioContext));
	std::thread echoThread(&WindowApp::clientEcho, &(*this));
	echoThread.detach();
	currentState = appRunning;

}


void WindowApp::processInput() {
	sf::Event ev;

	while (mainWindow->pollEvent(ev)) {
		//Window is closed via mouse or escape is pressed
		if (ev.type == sf::Event::Closed || 
			(ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape)) 
		{
			this->mainWindow->close();
		}
		//TODO refactor this shit
		if (currentState == appRunning)return;

		for (auto i : uiElements) {
			if (i->elementFunction(ev)) {
				switch (i->getId())
				{
				case 1:
					startClient();
					break;
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


void WindowApp::renderElements() {

	if (gameServer != nullptr)
		return;
	for (uiElement *i : uiElements) {
		i->renderElement(mainWindow);
	}

}


int WindowApp::main() {

	while (mainWindow->isOpen()) {
		this->processInput();
		mainWindow->clear(sf::Color::Black);

		renderElements();
		mainWindow->display();
	}
	return 0;
}


