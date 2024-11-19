#pragma once
#include <list>
#include <memory>
#include <vector>
#include "SFML/Graphics.hpp"
#include "../Networking/Server/Server.hpp"
#include "../Networking/Client/Client.hpp"
#include "AppState.hpp"
#include "Menu/Button.hpp"

class WindowApp {
private:
    unsigned width, height;
    sf::RenderWindow* mainWindow;
    AppState currentState;
    asio::io_context& ioContext;

    std::vector<std::unique_ptr<uiElement>> uiElements;

    // Állapot inicializálás
    void initializeMenu();
    void initializePlayState();
    void initializeSettingsState();
    void initializeJoinState();
    void initializeLobbyStateHost();
    void initializeLobbyStateClient();

    // Memóriakezelés
    void clearUIElements();

public:
    WindowApp(asio::io_context& ioC, const unsigned width = 800, const unsigned height = 600);
    ~WindowApp();

    void processInput();
    void renderElements();
    int main();
};
