#include "WindowApp.hpp"
#include <iostream>

WindowApp::WindowApp(asio::io_context& ioC, const unsigned width, const unsigned height): width(width), height(height), mainWindow(nullptr), currentState(AppState::MENU), ioContext(ioC) {
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


bool WindowApp::startClient() {
    if (this->player != nullptr)return false;
    currentState = AppState::GAME;
    this->player = std::unique_ptr<Client>(new Client("localhost", 8085, ioContext));
    if (this->player->connect() == "") {
        return false;
    }

    this->player->setGameStateCallback([this](const json& gameState) {
        this->processGameState(gameState); // A játékállapot feldolgozása
        });

    std::thread t(&Client::waitForGame, player.get());
    t.detach();

    return true;
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
    if (player != nullptr) {
        try {
            player->sendDisconnect();
            Client * p = player.release();
            delete p;
            p = nullptr;
        }
        catch (std::exception e) {
            logErr(e.what());
        }
    }
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
    if (server) {
        server->setState(serverStateGameStart);
        server->setupGameState();
    }
    if (player) {
        player->setState(cStateRunGame);
    }
    currentState = AppState::GAME;
}


void WindowApp::playerReady() {
    currentState = AppState::LOBBY_CLIENT_READY;
    clearUIElements();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 300), sf::Vector2f(125, 50), "Waiting...", 16U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(width / 2 - 100, 400), sf::Vector2f(125, 50), "Quit", 15U));
    player->sendReady(true);
    std::thread t = std::thread(&Client::waitForGame, &(*player.get()));
    t.detach();
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
                    case 5: { //Host clicks on "host" button
                        startServer(); 
                        initializeLobbyStateHost(); 
                        break;
                    }
                    case 6: initializeMenu(); break;
                    case 7: initializeMenu(); break;
                    case 10:{ 
                        startClient() ? initializeLobbyStateClient() : initializeMenu();
                        break; 
                    }
                    case 11: initializePlayState(); break;
                    case 12: currentState = AppState::GAME; startGame();  break; // Start the game
                    case 13: initializeMenu(); server->shutDown(); break; // Quit lobby - host
                    case 14: playerReady(); break; //Client ready in lobby
                    case 15: initializeMenu(); break; // Quit lobby - client
                default: break;
                }
                break;
            }
        }
        if (currentState == AppState::LOBBY_CLIENT_READY && player->getState()==cStateStartGame) {
            clearUIElements();
        }
    }
}   

// Tömörített map visszaállítása
std::vector<std::vector<int>> WindowApp::decompressMap(const std::vector<std::vector<std::pair<int, int>>>& compressedMap) {
    std::vector<std::vector<int>> decompressedMap;
    for (const auto& compressedRow : compressedMap) {
        std::vector<int> row;
        for (const auto& [value, count] : compressedRow) {
            row.insert(row.end(), count, value); // Az értékek kibontása
        }
        decompressedMap.push_back(row);
    }
    return decompressedMap;
}

void WindowApp::processGameState(const json& gameState) {
    // Térkép betöltése
    auto compressedMap = gameState["map"].get<std::vector<std::vector<std::pair<int, int>>>>();
    maze = decompressMap(compressedMap); // Map visszaállítása

    // Feladatok betöltése
    tasks.clear();
    for (const auto& taskJson : gameState["tasks"]) {
        Task task(taskJson["position"][0], taskJson["position"][1]);
        task.progress = taskJson["progress"];
        tasks.push_back(task);
    }

    // Túlélők betöltése
    survivors.clear();
    for (const auto& survivorJson : gameState["players"]) {
        Survivor survivor(survivorJson["position"][0], survivorJson["position"][1],
            { sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right });
        survivor.healthState = survivorJson["healthState"];
        survivor.speedBoostTimer = survivorJson["speedBoostTimer"];
        survivor.dyingTimer = survivorJson["dyingTimer"];
        survivors.push_back(survivor);
    }

    // Gyilkos betöltése
    killer = Killer(gameState["killer"]["position"][0], gameState["killer"]["position"][1],
        { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D });
}

void WindowApp::renderGameState() {
    mainWindow->clear(sf::Color::Black);

    // Pálya megjelenítése
    renderMap(*mainWindow, maze, killer, survivors, true);

    // Feladatok megjelenítése
    for (const auto& task : tasks) {
        task.render(*mainWindow, true, false);
    }

    // Játékosok megjelenítése
    for (const auto& survivor : survivors) {
        survivor.render(*mainWindow);
    }
    killer.render(*mainWindow);

    mainWindow->display();
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

        if (currentState == AppState::GAME) {
            renderGameState(); // Játék renderelése
        }
        else {
            renderElements(); // Egyéb UI elemek renderelése
        }

        ioContext.poll();
        std::this_thread::sleep_for(16ms); // ~60 FPS
    }
    return 0;
}

