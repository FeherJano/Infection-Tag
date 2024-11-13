#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "AppState.hpp"
#include "../Server/server.hpp"
#include "../Client/Client.hpp"
#include "Menu/Button.hpp"


class WindowApp {
public:
    WindowApp(const unsigned width = 1024, const unsigned height = 768);
    ~WindowApp();

    void startServer();
    void startClient();
    void clientEcho();
    int main();

private:
    void processInput();
    void renderElements();

    void initializeMenu();
    void initializePlayState();
    void initializeLobbyStateHost();
    void initializeLobbyStateClient();
    void initializeJoinState();

    sf::RenderWindow* mainWindow;
    std::vector<uiElement*> uiElements;
    AppState currentState;

    unsigned width;
    unsigned height;
    std::unique_ptr<CatGameServer> gameServer;
    std::unique_ptr<Client> player;
    const std::string localhost = "127.0.0.1";
    const unsigned short myPort = 54000;
};
