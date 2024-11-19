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

// clk
void WindowApp::processInput() {
    sf::Event ev;
    while (mainWindow->pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) {
            mainWindow->close();
            return;
        }

        for (auto& element : uiElements) {
            if (!element) continue; // magic fix
            if (element->elementFunction(ev)) {
                switch (element->getId()) {
                case 1: initializePlayState(); break; // Play
                case 2: initializeSettingsState(); break; // Settings
                case 3: mainWindow->close(); break; // Exit
                case 4: /* TODO: Join logic */ break; // Join
                case 5: /* TODO: Host logic */ break; // Host
                case 6: initializeMenu(); break; // Back to Menu
                case 7: initializeMenu(); break; // Back from Settings
                default: break;
                }
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
