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



void WindowApp::processGameData(const json& gameData,
    std::vector<std::vector<int>>& maze,
    std::vector<Survivor>& survivors,
    Killer& killer,
    std::vector<Task>& tasks) {

    // Térkép feldolgozása
    if (gameData.contains("map") && gameData["map"].is_array()) {
        try {
            auto compressedMap = gameData["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
            if (client) {
                maze = client->decompressMap(compressedMap);
            }
            else {
                std::cerr << "Client instance is null. Cannot decompress map." << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error processing 'map': " << e.what() << std::endl;
        }
    } else {
        std::cerr << "'map' key is missing or not an array." << std::endl;
    }

    // Feladatok feldolgozása
    tasks.clear();
    if (gameData.contains("tasks") && gameData["tasks"].is_array()) {
        for (const auto& taskJson : gameData["tasks"]) {
            try {
                Task task(0, 0);
                task.from_json(taskJson);
                tasks.push_back(task);
            } catch (const std::exception& e) {
                std::cerr << "Error processing task: " << e.what() << " Data: " << taskJson.dump() << std::endl;
            }
        }
    } else {
        std::cerr << "'tasks' key is missing or not an array." << std::endl;
    }

    // Játékosok feldolgozása
    survivors.clear();
    if (gameData.contains("players") && gameData["players"].is_array()) {
        for (const auto& playerJson : gameData["players"]) {
            try {
                Survivor survivor(0, 0, { sf::Keyboard::Unknown, sf::Keyboard::Unknown, sf::Keyboard::Unknown, sf::Keyboard::Unknown });
                survivor.from_json(playerJson);
                survivors.push_back(survivor);
            } catch (const std::exception& e) {
                std::cerr << "Error processing player: " << e.what() << " Data: " << playerJson.dump() << std::endl;
            }
        }
    } else {
        std::cerr << "'players' key is missing or not an array." << std::endl;
    }

    // Gyilkos feldolgozása
    if (gameData.contains("killer") && gameData["killer"].is_object()) {
        try {
            killer.from_json(gameData["killer"]);
        } catch (const std::exception& e) {
            std::cerr << "Error processing killer: " << e.what() << " Data: " << gameData["killer"].dump() << std::endl;
        }
    } else {
        std::cerr << "'killer' key is missing or not an object." << std::endl;
    }
}

void WindowApp::renderGame(const json& gameData, bool showFullMap) {
    std::vector<std::vector<int>> maze;
    std::vector<Survivor> survivors;
    Killer killer(0, 0, { sf::Keyboard::Unknown, sf::Keyboard::Unknown, sf::Keyboard::Unknown, sf::Keyboard::Unknown });
    std::vector<Task> tasks;

    // Game data feldolgozása
    processGameData(gameData, maze, survivors, killer, tasks);

    // Renderelés
    mainWindow->clear();
    renderMap(*mainWindow, maze, killer, survivors, showFullMap);
    for (const auto& survivor : survivors) {
        survivor.render(*mainWindow);
    }
    killer.render(*mainWindow);
    for (const auto& task : tasks) {
        bool isVisible = isTaskVisibleToAnySurvivor(task, survivors, maze);
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
                renderGame(client->getGameData(), false);
            }
            else if (server) {
                renderGame(server->gameData, true);
            }
        }
        else {
            renderElements();
        }
    }
    return 0;
}


