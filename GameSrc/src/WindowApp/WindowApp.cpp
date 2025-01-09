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
    reset();
}

void WindowApp::initializeMenu() {
    if (!this->menuBackgroundTex.loadFromFile("./src/WindowApp/Textures/menu1.jpg")) {
        std::cout << "ERROR::MENU::COULD NOT LOAD BACKGROUND TEXTURE" << "\n";
    }
    this->menuBackground.setTexture(this->menuBackgroundTex);
    uiElements.clear();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 400), sf::Vector2f(200, 50), "Host", 1U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 500), sf::Vector2f(200, 50), "Join", 2U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 600), sf::Vector2f(200, 50), "Exit", 3U));
    currentState = AppState::MENU;
}

void WindowApp::initializeLobby() {
    uiElements.clear();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 400), sf::Vector2f(200, 50), "Start Game", 4U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 500), sf::Vector2f(200, 50), "Back", 5U));
    currentState = AppState::LOBBY;
}

void WindowApp::initializeConnectionState() {
    uiElements.clear();
    uiElements.push_back(std::make_unique<TextBox>(*mainWindow, sf::Vector2f(300, 500), sf::Vector2f(200, 50), "IP Address", 6U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 600), sf::Vector2f(80, 50), "Back", 5U));
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(420, 600), sf::Vector2f(80, 50), "Join", 7U));
    currentState = AppState::CONNECT;
}

void WindowApp::initializeClientLobby() {
    uiElements.clear();
    uiElements.push_back(std::make_unique<Button>(*mainWindow, sf::Vector2f(300, 400), sf::Vector2f(200, 50), "Waiting...", 9U));
    currentState = AppState::LOBBY;
}


void WindowApp::intitalizeCatWin() {
    uiElements.clear();
    if (!this->menuBackgroundTex.loadFromFile("./src/WindowApp/Textures/catWin.jpg")) {
        std::cout << "ERROR::MENU::COULD NOT LOAD BACKGROUND TEXTURE" << "\n";
    }
    this->menuBackground.setTexture(this->menuBackgroundTex);
    currentState = AppState::END;
}


void WindowApp::initializeRatWin() {
    uiElements.clear();
    if (!this->menuBackgroundTex.loadFromFile("./src/WindowApp/Textures/ratWin.jpg")) {
        std::cout << "ERROR::MENU::COULD NOT LOAD BACKGROUND TEXTURE" << "\n";
    }
    this->menuBackground.setTexture(this->menuBackgroundTex);
    currentState = AppState::END;
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

bool WindowApp::startClient(std::string address) {
    client = std::make_unique<Client>(address, 8085, ioContext);
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

void WindowApp::updateDirection() {
    sf::Vector2f direction(0.0f, 0.0f);

    // Eltávolítjuk azokat a gombokat, amelyek már nincsenek lenyomva
    for (auto it = inputState.begin(); it != inputState.end();) {
        if (!sf::Keyboard::isKeyPressed(*it)) {
           // std::cout << "Removing key from inputState: " << *it << std::endl;
            it = inputState.erase(it);
        }
        else {
            ++it;
        }
    }

    // Az inputState alapján számoljuk az irányt
    if (inputState.count(sf::Keyboard::W)) direction.y -= 1.0f;
    if (inputState.count(sf::Keyboard::S)) direction.y += 1.0f;
    if (inputState.count(sf::Keyboard::A)) direction.x -= 1.0f;
    if (inputState.count(sf::Keyboard::D)) direction.x += 1.0f;

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0) {
        direction.x /= length;
        direction.y /= length;
    }

    // Küldjük el a szervernek az irányt
    if (client) {
        client->sendPlayerInput(direction);
        //std::cout << "sent direction: " << direction.x << ", " << direction.y << std::endl;
    }
}


void WindowApp::processInput() {
    sf::Event event;
    sf::Vector2f direction(0.0f, 0.0f);
    std::string address;

    while (mainWindow->pollEvent(event)) {
        // Ablak bezárása vagy kilépés ESC gombbal
        if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            mainWindow->close();
            return;
        }
        
        // Játékbeli mozgás érzékelése
        if (currentState == AppState::GAME && event.type == sf::Event::KeyPressed) {
            //std::cout << "button event Id: " << event.key.code << std::endl;

            if (event.type == sf::Event::KeyPressed) {
                inputState.insert(event.key.code);
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
                    //std::cout << "UI Element triggered with ID: " << element->getId() << std::endl;
                    switch (element->getId()) {
                    case 1: // Szerver indítása és lobby megnyitása
                        startServer();
                        initializeLobby();
                        break;
                    case 2: // Csatlakozási adatok megadása
                        initializeConnectionState();
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
                    case 6:
                        address = element->getText();
                        break;
                    case 7: // Kliens csatlakozása és lobby megnyitása
                        if (startClient(address)) {
                            initializeClientLobby();
                        }
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
            Survivor survivor(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D });
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

    if (message.contains("type") && message["type"] == "endgame") {
        isEndgame = true;

        float gameTime = message["duration"];
        endgameMessage = "Game Over\nMice survived for " + std::to_string(static_cast<int>(gameTime)) + " seconds.";

        std::cout << "Game over message processed. Duration: " << gameTime << " seconds." << std::endl;
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
            Survivor survivor(0, 0, { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D });
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
                //std::cout << "Client ID does not match killer playerId." << std::endl;
            }
        }
        else {
            //std::cout << "Killer data does not contain playerId." << std::endl;
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
    mainWindow->draw(this->menuBackground);
    for (const auto& element : uiElements) {
        element->renderElement(mainWindow);
    }
    mainWindow->display();
}

void WindowApp::renderEndgameScreen() {
    sf::Text endgameText;
    sf::Font font;

    if (!font.loadFromFile("./src/Utility/Fonts/Raleway-Regular.ttf")) {
        std::cerr << "Failed to load font: Raleway-Regular.ttf" << '\n';
        exit(-1);
    }

    endgameText.setFont(font);
    endgameText.setString(endgameMessage);
    endgameText.setCharacterSize(50);
    endgameText.setFillColor(sf::Color::White);
    endgameText.setOutlineThickness(2);
    endgameText.setOutlineColor(sf::Color::Black);
    endgameText.setStyle(sf::Text::Bold);

    sf::FloatRect textBounds = endgameText.getLocalBounds();
    endgameText.setPosition((width - textBounds.width) / 2, (height - textBounds.height) / 2);

    mainWindow->clear();
    mainWindow->draw(endgameText);
    mainWindow->display();
    
}


int WindowApp::main() {
    sf::Clock clock; // Időmérés a deltaTime-hoz

    while (mainWindow->isOpen()) {
        processInput(); // Események kezelése

        float deltaTime = clock.restart().asSeconds(); // DeltaTime kiszámítása

        if (isEndgame) {
            renderEndgameScreen();
            break;
        }

        if (currentState == AppState::LOBBY && client && client->isGameDataReady()) {
            initializeGame(); // Átváltunk GAME állapotra
        }

        if (currentState == AppState::GAME) {
            sf::Event event{};
            if (client) {

                if (inputState.size() > 0) {
                    updateDirection(); // Aktuális irány frissítése
                }

                

                // Karakter mozgás frissítése deltaTime alapján
                //client->getPlayer()->move(deltaTime, maze);

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


