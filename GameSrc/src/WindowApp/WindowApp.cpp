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

void WindowApp::initializeClientLobby() {
    uiElements.clear();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 300), sf::Vector2f(200, 50), "Waiting for Server...", 6U));
    currentState = AppState::LOBBY;
}


void WindowApp::initializeGame() {
    uiElements.clear();
    currentState = AppState::GAME;
}

void WindowApp::startServer() {
    // Szerver indítása
    server = std::make_unique<CatGameServer>(ioContext, 8085);
    server->setState(serverStateLobby);

    // Szerver listen indítása külön szálon
    std::thread([this]() {
        std::cout << "Server started. Waiting for messages on port 8085..." << std::endl;
        server->ServerFunction();
        }).detach();

    // Host kliens csatlakoztatása
    client = std::make_unique<Client>("localhost", 8085, ioContext);
    std::cout << "Starting local Client for Host..." << std::endl;

    if (client->connect() != "") {
        //std::cout << "Local Client Connected as Host." << std::endl;

        std::thread([this]() {
            client->cListen([this](const json& message) {
                processIncomingMessage(message, *client->getPlayer());
                });
            }).detach();
    }
    else {
        std::cerr << "Failed to connect local Client for Host." << std::endl;
    }
}

bool WindowApp::startClient() {
    client = std::make_unique<Client>("localhost", 8085, ioContext);
    std::cout << "Starting Client " << std::endl;

    if (client->connect() != "") {
        std::cout << "Client Connected" << std::endl;

        std::thread([this]() {
            client->cListen([this](const json& message) {
                processIncomingMessage(message, *client->getPlayer());
                });
            }).detach();

        return true;
    }
    std::cout << "Starting Client Failed " << std::endl;
    return false;
}


void WindowApp::processInput() {
    sf::Event event;
    sf::Vector2f direction(0.0f, 0.0f);

    while (mainWindow->pollEvent(event)) {
        // Ablak bezárása vagy kilépés ESC gombbal
        if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            mainWindow->close();
            return;
        }

        // Játékbeli mozgás érzékelése
        if (currentState == AppState::GAME && event.type == sf::Event::KeyPressed) {
            std::cout << "button event Id: " << event.key.code << std::endl;
            if (event.key.code == sf::Keyboard::W) {
                direction.y -= 1.0f;
            }
            if (event.key.code == sf::Keyboard::S) {
                direction.y += 1.0f;
            }
            if (event.key.code == sf::Keyboard::A) {
                direction.x -= 1.0f;
            }
            if (event.key.code == sf::Keyboard::D) {
                direction.x += 1.0f;
            }

            if (client) {
                client->sendPlayerInput(direction);
                std::cout << "sent direction: " << direction.x << direction.y << std::endl;
            }
            if (event.key.code == sf::Keyboard::Q) {
                resetServer();
                return;
            }
        }

        // UI elemek kezelése
        if (currentState != AppState::GAME) {
            for (auto& element : uiElements) {
                if (element && element->elementFunction(event)) {
                    std::cout << "UI Element triggered with ID: " << element->getId() << std::endl;
                    switch (element->getId()) {
                    case 1: // Szerver indítása és lobby megnyitása
                        startServer();
                        initializeLobby();
                        break;
                    case 2: // Kliens csatlakozása és lobby megnyitása
                        if (startClient()) {
                            initializeClientLobby();
                        }
                        break;
                    case 3: // Kilépés
                        mainWindow->close();
                        break;
                    case 4: // Játék indítása
                        if (server) {
                            server->generateGameData();
                            server->broadcastGameData();
                            initializeGame();
                        }
                        break;
                    case 5: // Visszatérés a főmenübe
                        initializeMenu();
                        break;
                    default:
                        break;
                    }
                    break;
                }
            }
        }
    }
}


void WindowApp::processGameData(const json& gameData, std::vector<std::vector<int>>& maze, std::vector<Survivor>& survivors, Killer& killer, std::vector<Task>& tasks, Player& clientPlayer) {
    // Térkép feldolgozása
    if (gameData.contains("map") && gameData["map"].is_array()) {
        auto compressedMap = gameData["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
        if (client) {
            maze = client->decompressMap(compressedMap);
        }
    }

    // Gyilkos pozíciójának feldolgozása
    if (gameData.contains("killer")) {
        auto killerData = gameData["killer"];
        killer.from_json(killerData);

        // Ha a gyilkos a kliens játékosa, állítsuk be a pozíciót
        if (killerData.contains("playerId")) {
            if (client->getId() == killerData["playerId"]) {
                clientPlayer.setPosition(killerData["position"][0], killerData["position"][1]);
            }
        }
    }

    // Túlélők pozíciójának feldolgozása
    if (gameData.contains("players") && gameData["players"].is_array()) {
        survivors.clear();
        for (const auto& playerData : gameData["players"]) {
            Survivor survivor(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D }, playerData["playerId"]);
            survivor.from_json(playerData);
            survivors.push_back(survivor);

            // Ha ez a túlélő a kliens játékosa, állítsuk be a clientPlayer-t
            if (playerData.contains("playerId"))
            {
                if (client->getId() == playerData["playerId"]) {
                    clientPlayer.setPosition(playerData["position"][0], playerData["position"][1]);

                }
            }
        }
    }

    // Feladatok feldolgozása
    tasks.clear();
    if (gameData.contains("tasks")) {
        for (const auto& taskData : gameData["tasks"]) {
            Task task(0, 0);
            task.from_json(taskData);
            tasks.push_back(task);
        }
    }
}

void WindowApp::processIncomingMessage(const json& message, Player& clientPlayer) {
    //If server sends reset message, we destroy the game and reset to lobby
    if (message.contains("type") && message["type"] == "reset") {
        currentState = AppState::MENU;
        this->reset();
        return;
    }


    if (message.contains("map")) {
        auto compressedMap = message["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
        auto decompressedMap = client->decompressMap(compressedMap);
        // Térkép frissítése
        maze = decompressedMap;
    }

    if (message.contains("players")) {
        survivors.clear();
        for (const auto& playerData : message["players"]) {
            Survivor survivor(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D }, playerData["playerId"]);
            survivor.from_json(playerData);
            survivors.push_back(survivor);

            if (playerData.contains("playerId"))
            {
                if (client->getId() == playerData["playerId"]) {
                    clientPlayer.setPosition(playerData["position"][0], playerData["position"][1]);

                }
            }
        }
    }

    if (message.contains("killer")) {
        auto killerData = message["killer"];
        killer.from_json(killerData);

        if (killerData.contains("playerId")) {

            if (client->getId() == killerData["playerId"]) {
                clientPlayer.setPosition(killerData["position"][0], killerData["position"][1]);
            }
            else {
                std::cout << "Client ID does not match killer playerId." << std::endl;
            }
        }
        else {
            std::cout << "Killer data does not contain playerId." << std::endl;
        }
    }

}



void WindowApp::renderGame(const json& gameData, Player& clientPlayer, bool showFullMap) {

    if (!isInitialized) {
        std::cout << "Running processGameData for initialization..." << std::endl;
        processGameData(gameData, maze, survivors, killer, tasks, clientPlayer);
        std::cout << "Initialization completed." << std::endl;
        isInitialized = true;
    }

    // Renderelés
    mainWindow->clear();

    // Térkép kirajzolása
    renderMap(*mainWindow, maze, clientPlayer, survivors, killer, showFullMap);

    // Feladatok kirajzolása
    for (const auto& task : tasks) {
        bool isVisible = isCellVisible(clientPlayer.position, task.position.x / CELL_SIZE, task.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze);
        task.render(*mainWindow, isVisible, showFullMap);
    }

    mainWindow->display();
}


//Calls server's destructor which will broadcast a reset call to all clients.
void WindowApp::resetServer() {
    currentState = AppState::MENU;
    std::this_thread::sleep_for(20ms);
    if (server) {
        //deleting server
        server.reset();
        server = nullptr;
    }
    std::cout << "Server shut down successfully!" << std::endl;
}

//Resets the entire game, and exits to menu, invoked by server's reset call.
void WindowApp::reset() {
    currentState = AppState::MENU;
    std::this_thread::sleep_for(100ms);
    if (client) {
        client.reset();
        client = nullptr;
    }
    uiElements.clear();
    survivors.clear();
    maze.clear();
    tasks.clear();

    initializeMenu();
    std::cout << "Reset complete succesfully!" << std::endl;

}

void WindowApp::renderElements() {
    mainWindow->clear();
    for (const auto& element : uiElements) {
        element->renderElement(mainWindow);
    }
    mainWindow->display();
}



int WindowApp::main() {
    while (mainWindow->isOpen()) {
        processInput();

        if (currentState == AppState::LOBBY && client && client->isGameDataReady()) {
            initializeGame(); // Átváltunk GAME állapotra
        }

        if (currentState == AppState::GAME) {
            if (client) {
                renderGame(client->getGameData(), *client->getPlayer(), false);
            }
            else if (server) {
                //renderGame(server->gameData, false);
            }
        }
        else {
            renderElements();
        }
    }
    return 0;
}


