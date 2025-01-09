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
#include "Menu/TextBox.hpp"
#include <mutex>
#include <set>

enum class AppState {
    MENU,
    GAME,
    CONNECT,
    LOBBY,
    END
};


class WindowApp {
public:
    WindowApp(asio::io_context& ioContext, unsigned width, unsigned height);
    ~WindowApp();

    int main();
    std::string address = "";
private:
    sf::RenderWindow* mainWindow;
    unsigned width, height;
    AppState currentState;

    sf::Texture menuBackgroundTex;
    sf::Sprite menuBackground;

    std::vector<std::unique_ptr<uiElement>> uiElements;
    asio::io_context& ioContext;

    std::unique_ptr<CatGameServer> server;
    std::unique_ptr<Client> client;

    std::vector<Survivor> survivors;
    Killer killer;
    std::vector<std::vector<int>> maze;
    std::vector<Task> tasks;
    std::set<sf::Keyboard::Key> inputState;

    bool isEndgame = false;
    std::string endgameMessage;


    bool isInitialized = false;

    void initializeMenu();
    void initializeLobby();
    void initializeConnectionState();
    void initializeClientLobby();
    void initializeGame();
    void intitalizeCatWin();
    void initializeRatWin();

    void processInput();
    void updateDirection();
    void renderEndgameScreen();
    void renderElements();
    void renderGame(const json& gameData, Player& clientPlayer, bool showFullMap);
    void processIncomingMessage(const json& message, Player& clientPlayer); // JSON üzenet feldolgozása

    void processGameData(const json& gameData,
        std::vector<std::vector<int>>& maze,
        std::vector<Survivor>& survivors,
        Killer& killer,
        std::vector<Task>& tasks,
        Player& clientPlayer);

    void startServer();
    bool startClient(std::string address);
    void reset();
    void resetServer();

};

#endif
