#include "WindowApp.hpp"
#include <iostream>

WindowApp::WindowApp(asio::io_context& ioC, const unsigned width, const unsigned height)
    : width(width), height(height), mainWindow(nullptr), currentState(AppState::MENU), ioContext(ioC) {
    mainWindow = new sf::RenderWindow(sf::VideoMode(width, height), "Freaking Cats");
    initializeMenu();
}

WindowApp::~WindowApp() {
    clearUIElements();
    if (mainWindow) {
        mainWindow->close();
        delete mainWindow;
    }
}

void WindowApp::startServer() {
    if (this->server != nullptr)return;
    this->server = std::unique_ptr<CatGameServer>(new CatGameServer(ioContext, myPort));
    std::thread serverThread(&CatGameServer::ServerFunction, &(*server));
    serverThread.detach();
    server->setState(serverStateLobby);
}


void WindowApp::startClient() {
    if (this->player != nullptr)return;
    currentState = AppState::GAME;
    this->player = std::unique_ptr<Client>(new Client("localhost", 8085, ioContext));
    if (this->player->connect() == "") {
        initializeMenu();
        currentState = AppState::MENU;
    }
}

void WindowApp::clearUIElements() {
    uiElements.clear();
}

// Menu state
void WindowApp::initializeMenu() {
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 100), sf::Vector2f(125, 50), "Play", 1U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 200), sf::Vector2f(125, 50), "Settings", 2U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 300), sf::Vector2f(125, 50), "Exit", 3U));
    currentState = AppState::MENU;
}

// Play state
void WindowApp::initializePlayState() {
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 100), sf::Vector2f(125, 50), "Join", 4U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 200), sf::Vector2f(125, 50), "Host", 5U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 300), sf::Vector2f(125, 50), "Back", 6U));
    currentState = AppState::PLAY;
}

// Settings state
void WindowApp::initializeSettingsState() {
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 300), sf::Vector2f(125, 50), "Back", 7U));
    currentState = AppState::SETTINGS;
}

// Join state
void WindowApp::initializeJoinState() {
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 150, 100), sf::Vector2f(200, 50), "Code", 8U)); // Text input field placeholder
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 150, 200), sf::Vector2f(200, 50), "IP", 9U));   // Text input field placeholder
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 140, 300), sf::Vector2f(190, 50), "JoinToLobby", 10U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 400), sf::Vector2f(125, 50), "Back", 11U));
    currentState = AppState::JOIN;
}

// Lobby Host state
void WindowApp::initializeLobbyStateHost() {
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 400), sf::Vector2f(125, 50), "Start", 12U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 500), sf::Vector2f(125, 50), "Quit", 13U));
    currentState = AppState::LOBBY_HOST;
}

// Lobby Client state
void WindowApp::initializeLobbyStateClient() {
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 400), sf::Vector2f(125, 50), "Ready", 14U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 500), sf::Vector2f(125, 50), "Quit", 15U));
    currentState = AppState::LOBBY_CLIENT;
}

void WindowApp::startGame() {
    server->setState(serverStateGameStart);
}


// clk
void WindowApp::processInput() {
    sf::Event ev;
    while (mainWindow->pollEvent(ev)) {
        if (ev.type == sf::Event::Closed || (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape)) {
            mainWindow->close();
            return;
        }
        for (auto& i : uiElements) {
            if (i && i->elementFunction(ev)) {
                std::cout << "UI Element triggered with ID: " << i->getId() << std::endl;
                switch (i->getId()) {
                    case 1: initializePlayState(); break;
                    case 2: initializeSettingsState(); break;
                    case 3: mainWindow->close(); break;
                    case 4: initializeJoinState(); break;
                    case 5: startServer(); initializeLobbyStateHost(); break;
                    case 6: initializeMenu(); break;
                    case 7: initializeMenu(); break;
                    case 10:{ startClient();  initializeLobbyStateClient(); break; }
                    case 11: initializePlayState(); break;
                    case 12: currentState = AppState::GAME; startGame();  break; // Start the game
                    case 13: initializeMenu(); break; // Quit lobby - host
                    case 14: /* TODO: Implement ready state logic */ break;
                    case 15: initializeMenu(); break; // Quit lobby - client
                default: break;
                }
                break;
            }
        }
    }
}

void WindowApp::renderElements() {
    mainWindow->clear(sf::Color::Black);
    for (const auto& element : uiElements) {
        if (element) {
            element->renderElement(mainWindow);
        }
    }
    mainWindow->display();
}

int WindowApp::main() {
    while (mainWindow->isOpen()) {
        processInput();
        renderElements();
        ioContext.poll();
    }
    return 0;
}
