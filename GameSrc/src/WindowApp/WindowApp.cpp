#include "WindowApp.hpp"
#include <iostream>

WindowApp::WindowApp(asio::io_context& ioContext, unsigned width, unsigned height)
    : ioContext(ioContext), width(width), height(height), mainWindow(nullptr), currentState(AppState::MENU) {
    mainWindow = new sf::RenderWindow(sf::VideoMode(width, height), "Multiplayer Game");
    initializeMenu();
}

WindowApp::~WindowApp() {
    if (mainWindow) {
        mainWindow->close();
        delete mainWindow;
    }
}

void WindowApp::initializeMenu() {
    uiElements.clear();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 200), sf::Vector2f(200, 50), "Host", 1U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 300), sf::Vector2f(200, 50), "Join", 2U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 400), sf::Vector2f(200, 50), "Exit", 3U));
    currentState = AppState::MENU;
}

void WindowApp::initializeLobby() {
    uiElements.clear();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 200), sf::Vector2f(200, 50), "Start Game", 4U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 300), sf::Vector2f(200, 50), "Back", 5U));
    currentState = AppState::LOBBY;
}

void WindowApp::initializeGame() {
    uiElements.clear();
    currentState = AppState::GAME;
}

void WindowApp::startServer() {
    server = std::make_unique<CatGameServer>(ioContext, 8085);
    server->setState(serverStateLobby);

    // Szerver listen indítása külön szálon
    std::thread([this]() {
        std::cout << "Server started. Waiting for messages on port 8085..." << std::endl;
        server->ServerFunction();
        }).detach();
}


bool WindowApp::startClient() {
    client = std::make_unique<Client>("localhost", 8085, ioContext);
    std::cout << "Starting Client " << std::endl;

    if (client->connect() != "") {
        std::cout << "Client Connected" << std::endl;
        std::thread([this]() {
            client->waitForGameData(); // Játékadatok fogadása
            }).detach();
        return true;
    }
    std::cout << "Starting Client Failed " << std::endl;
    return false;
}


void WindowApp::processInput() {
    sf::Event event;
    while (mainWindow->pollEvent(event)) {
        if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            mainWindow->close();
            return;
        }

        for (auto& element : uiElements) {
            if (element && element->elementFunction(event)) {
                std::cout << "UI Element triggered with ID: " << element->getId() << std::endl;
                switch (element->getId()) {
                case 1: startServer(); initializeLobby(); break;
                case 2: if (startClient()) initializeGame(); break;
                case 3: mainWindow->close(); break;
                case 4:
                    if (server) {
                        server->generateGameData(); // Játékadatok generálása
                        server->broadcastGameData(); // Játékadatok elküldése
                        initializeGame(); // Állapot váltás
                    }
                    break;
                case 5: initializeMenu(); break;
                default: break;
                }
                break;
            }
        }
    }
}



void WindowApp::renderElements() {
    mainWindow->clear();
    for (const auto& element : uiElements) {
        element->renderElement(mainWindow);
    }
    mainWindow->display();
}

void WindowApp::renderGame() {
    mainWindow->clear();
    // Add game rendering logic here
    mainWindow->display();
}

int WindowApp::main() {
    while (mainWindow->isOpen()) {
        processInput();

        if (currentState == AppState::GAME) {
            renderGame();
        }
        else {
            renderElements();
        }
    }
    return 0;
}
