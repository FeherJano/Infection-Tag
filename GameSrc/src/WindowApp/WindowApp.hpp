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

const unsigned short myPort = 8085;
const std::string localhost = "localhost";

class WindowApp {
protected:

    std::unique_ptr<CatGameServer> server;
    std::unique_ptr<Client> player;

    unsigned width, height;
    sf::RenderWindow* mainWindow;
    AppState currentState;
    asio::io_context& ioContext;

    std::vector<std::unique_ptr<uiElement>> uiElements;

    std::vector<std::vector<int>> maze;
    std::vector<Task> tasks;
    std::vector<Survivor> survivors;
    Killer killer;


private:

    // State inits
    void initializeMenu();
    void initializePlayState();
    void initializeSettingsState();
    void initializeJoinState();
    void initializeLobbyStateHost();
    void initializeLobbyStateClient();
    void startGame();

    void clearUIElements();

    void processInput();
    void renderElements();
    void renderGameState();
    void playerReady();

    std::vector<std::vector<int>> decompressMap(const std::vector<std::vector<std::pair<int, int>>>& compressedMap);

public:
    WindowApp(asio::io_context& ioC, const unsigned width = 800, const unsigned height = 600);
    ~WindowApp();

    void processGameState(const json& gameState);

    void startServer();
    bool startClient();

    int main();
};
