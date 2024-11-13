#include "WindowApp.hpp"
#include <iostream>
#include <thread>


WindowApp::WindowApp(const unsigned width = 1024, const unsigned height = 768) : width(width), height(height), gameServer(nullptr), player(nullptr) {
	mainWindow = new sf::RenderWindow(sf::VideoMode(width, height), "Freaking cats");

    initializeMenu();
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
    currentState = AppState::GAME;

}

void WindowApp::processInput() {
    sf::Event ev;
    while (mainWindow->pollEvent(ev)) {
        if (ev.type == sf::Event::Closed || (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape)) {
            mainWindow->close();
        }
        for (auto i : uiElements) {
            if (i->elementFunction(ev)) {
                switch (i->getId()) {
                case 1: initializePlayState(); break;
                case 2: /* Settings - TODO */ break;
                case 3: mainWindow->close(); break;
                case 4: initializeJoinState(); break;
                case 5: startServer(); initializeLobbyStateHost(); break;
                case 8: startClient(); initializeLobbyStateClient(); break;
                case 9: currentState = AppState::GAME; break;
                case 10: initializeMenu(); break;
                case 11: /* Ready button - TODO */ break;
                default: break;
                }
            }
        }
    }
}

void WindowApp::renderElements() {
    mainWindow->clear(sf::Color::Black);
    for (uiElement* i : uiElements) {
        i->renderElement(mainWindow);
    }
}

void WindowApp::initializeMenu() {
    uiElements.clear();
    uiElement* playButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 100), sf::Vector2f(125, 50), "Play", 1U);
    uiElement* settingsButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 200), sf::Vector2f(125, 50), "Settings", 2U);
    uiElement* exitButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 300), sf::Vector2f(125, 50), "Exit", 3U);
    uiElements.push_back(playButton);
    uiElements.push_back(settingsButton);
    uiElements.push_back(exitButton);
    currentState = AppState::MENU;
}

void WindowApp::initializePlayState() {
    uiElements.clear();
    uiElement* joinButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 100), sf::Vector2f(125, 50), "Join", 4U);
    uiElement* hostButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 200), sf::Vector2f(125, 50), "Host", 5U);
    uiElements.push_back(joinButton);
    uiElements.push_back(hostButton);
    currentState = AppState::PLAY;
}

void WindowApp::initializeJoinState() {
    uiElements.clear();
    uiElement* codeInput = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 100), sf::Vector2f(200, 50), "Code", 6U);
    uiElement* ipInput = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 200), sf::Vector2f(200, 50), "IP", 7U);
    uiElement* joinButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 300), sf::Vector2f(125, 50), "Join", 8U);
    uiElements.push_back(codeInput);
    uiElements.push_back(ipInput);
    uiElements.push_back(joinButton);
    currentState = AppState::JOIN;
}

void WindowApp::initializeLobbyStateHost() {
    uiElements.clear();
    uiElement* startButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 400), sf::Vector2f(125, 50), "Start", 9U);
    uiElement* quitButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 500), sf::Vector2f(125, 50), "Quit", 10U);
    uiElements.push_back(startButton);
    uiElements.push_back(quitButton);
    currentState = AppState::LOBBY_HOST;
}

void WindowApp::initializeLobbyStateClient() {
    uiElements.clear();
    uiElement* readyButton = new Button(*mainWindow, sf::Vector2f(width / 2 - 100, 400), sf::Vector2f(125, 50), "Ready", 11U);
    uiElements.push_back(readyButton);
    currentState = AppState::LOBBY_CLIENT;
}

int WindowApp::main() {
    while (mainWindow->isOpen()) {
        processInput();
        renderElements();
        mainWindow->display();
    }
    return 0;
}
