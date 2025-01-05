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
            client->waitForGameData();
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
                case 2: if (startClient()) initializeClientLobby(); break; // Kliens lobby indítása
                case 3: mainWindow->close(); break;
                case 4:
                    if (server) {
                        server->generateGameData();
                        server->broadcastGameData();
                        initializeGame();
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



void WindowApp::processGameData(const json& gameData, std::vector<std::vector<int>>& maze, std::vector<Survivor>& survivors, Killer& killer, std::vector<Task>& tasks) {
    // Térkép feldolgozása
    if (gameData.contains("map") && gameData["map"].is_array()) {
        auto compressedMap = gameData["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
        if (client) {
            maze = client->decompressMap(compressedMap);
        }
    }

    // Gyilkos pozíciójának feldolgozása
    if (gameData.contains("killer")) {
        killer.from_json(gameData["killer"]);
    }

    // Túlélők pozíciójának feldolgozása
    if (gameData.contains("players") && gameData["players"].is_array()) {
        survivors.clear();
        for (const auto& playerData : gameData["players"]) {
            Survivor survivor(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D });
            survivor.from_json(playerData);
            survivors.push_back(survivor);
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

void WindowApp::renderGame(const json& gameData, const Player& clientPlayer, bool showFullMap) {
    std::vector<std::vector<int>> maze;
    std::vector<Survivor> survivors;
    Killer killer(0, 0, { sf::Keyboard::Unknown, sf::Keyboard::Unknown, sf::Keyboard::Unknown, sf::Keyboard::Unknown });
    std::vector<Task> tasks;

    // Game data feldolgozása
    processGameData(gameData, maze, survivors, killer, tasks);

    // Renderelés
    mainWindow->clear();

    // Térkép kirajzolása
    renderMap(*mainWindow, maze, clientPlayer, survivors, killer, showFullMap);

    // Karakterek kirajzolása
    for (const auto& survivor : survivors) {
        survivor.render(*mainWindow);
    }
    killer.render(*mainWindow);

    // Feladatok kirajzolása
    for (const auto& task : tasks) {
        bool isVisible = isCellVisible(clientPlayer.position, task.position.x / CELL_SIZE, task.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze);
        task.render(*mainWindow, isVisible, showFullMap);
    }

    mainWindow->display();
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


