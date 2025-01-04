#ifndef WINDOWAPP_HPP
#define WINDOWAPP_HPP

#pragma once
#include <list>
#include <memory>
#include <vector>
#include "SFML/Graphics.hpp"
#include "../Networking/Server/Server.hpp"
#include "../Networking/Client/Client.hpp"
#include "../Player/player.hpp"
#include "AppState.hpp"
#include "Menu/Button.hpp"
#include <mutex>

enum class AppState {
    MENU,
    GAME,
    LOBBY
};

class WindowApp {
public:
    WindowApp(asio::io_context& ioContext, unsigned width, unsigned height);
    ~WindowApp();

    int main();

private:
    sf::RenderWindow* mainWindow;
    unsigned width, height;
    AppState currentState;

    std::vector<std::unique_ptr<Button>> uiElements;
    asio::io_context& ioContext;

    std::unique_ptr<CatGameServer> server;
    std::unique_ptr<Client> client;

    void initializeMenu();
    void initializeLobby();
    void initializeGame();


    void processInput();
    void renderElements();
    void renderGame();

    void startServer();
    bool startClient();
};

#endif
