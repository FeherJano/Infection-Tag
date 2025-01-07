#include "map.hpp"
#include "structures.hpp"
#include "game_constants.hpp"
#include <random>
#include <iostream>


bool isBlocked(int gridX, int gridY, const std::vector<std::vector<int>>& maze) {
    if (gridX < 0 || gridX >= WIDTH || gridY < 0 || gridY >= HEIGHT) {
        return true;
    }
    return maze[gridY][gridX] != 0;
}

void placeObjects(std::vector<std::vector<int>>& maze) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> smallObjDist(0, SMALL_OBJECTS.size() - 1);
    uniform_int_distribution<> largeObjDist(0, LARGE_OBJECTS.size() - 1);
    uniform_int_distribution<> mainBuildingDist(0, MAIN_BUILDINGS.size() - 1);
    uniform_int_distribution<> rotDist(0, 3); // 0-3 rotation
    uniform_int_distribution<> blockDistX(0, GRID_COLS - 1);
    uniform_int_distribution<> blockDistY(0, GRID_ROWS - 1);

    // Block dimensions in rows and columns
    int blockWidth = WIDTH / GRID_COLS;
    int blockHeight = HEIGHT / GRID_ROWS;

    // List of blocks to track where objects have been placed
    vector<pair<int, int>> availableBlocks;

    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            availableBlocks.push_back(make_pair(i, j));
        }
    }

    // Place the main building randomly in a block
    int mainBuildingIndex = mainBuildingDist(gen);
    vector<vector<int>> mainBuilding = MAIN_BUILDINGS[mainBuildingIndex];
    int rotationsMainBuilding = rotDist(gen);
    for (int r = 0; r < rotationsMainBuilding; ++r) {
        mainBuilding = rotateRight(mainBuilding);
    }

    int blockIndex = rand() % availableBlocks.size();
    int mainBlockX = availableBlocks[blockIndex].second * blockWidth;
    int mainBlockY = availableBlocks[blockIndex].first * blockHeight;
    availableBlocks.erase(availableBlocks.begin() + blockIndex);

    int mainWidth = mainBuilding[0].size();
    int mainHeight = mainBuilding.size();
    int hx, hy;

    do {
        hx = mainBlockX + rand() % (blockWidth - mainWidth);
        hy = mainBlockY + rand() % (blockHeight - mainHeight);
    } while (!canPlaceObject(maze, hx, hy, mainWidth, mainHeight));
    
    for (int r = 0; r < mainHeight; ++r) {
        for (int c = 0; c < mainWidth; ++c) {
            if (mainBuilding[r][c] == 1) {
                maze[hy + r][hx + c] = mainBuilding[r][c];
            }
        }
    }

    // Place large objects in each block
    for (int i = 0; i < availableBlocks.size(); ++i) {
        int objIndex = largeObjDist(gen);
        vector<vector<int>> obj = LARGE_OBJECTS[objIndex];
        int rotations = rotDist(gen);
        for (int r = 0; r < rotations; ++r) {
            obj = rotateRight(obj);
        }

        int width = obj[0].size();
        int height = obj.size();

        int blockX = availableBlocks[i].second * blockWidth;
        int blockY = availableBlocks[i].first * blockHeight;

        int x, y;
        do {
            x = blockX + rand() % (blockWidth - width);
            y = blockY + rand() % (blockHeight - height);
        } while (!canPlaceObject(maze, x, y, width, height));

        for (int r = 0; r < height; ++r) {
            for (int c = 0; c < width; ++c) {
                if (obj[r][c] == 1) {
                    maze[y + r][x + c] = 1;
                }
            }
        }
    }

    // Place small objects randomly on the remaining space
    int numSmallObjects = 30;
    for (int i = 0; i < numSmallObjects; ++i) {
        int objIndex = smallObjDist(gen);
        vector<vector<int>> obj = SMALL_OBJECTS[objIndex];
        int rotations = rotDist(gen);
        for (int r = 0; r < rotations; ++r) {
            obj = rotateRight(obj);
        }

        int width = obj[0].size();
        int height = obj.size();

        int x, y;
        do {
            x = rand() % (WIDTH - width);
            y = rand() % (HEIGHT - height);
        } while (!canPlaceObject(maze, x, y, width, height));

        for (int r = 0; r < height; ++r) {
            for (int c = 0; c < width; ++c) {
                if (obj[r][c] == 1) {
                    maze[y + r][x + c] = 1;
                }
            }
        }
    }
}

void renderMap_og(sf::RenderWindow& window, const std::vector<std::vector<int>>& maze, const Killer& killer, const std::vector<Survivor>& survivors, bool showFullMap) {

    if (maze.size() < HEIGHT) {
        std::cerr << "Error: Maze has fewer rows than expected. Expected: " << HEIGHT << ", Actual: " << maze.size() << std::endl;
        return;
    }
    for (size_t i = 0; i < maze.size(); ++i) {
        if (maze[i].size() < WIDTH) {
            std::cerr << "Error: Maze row " << i << " has fewer columns than expected. Expected: " << WIDTH << ", Actual: " << maze[i].size() << std::endl;
            return;
        }
    }

    // A térkép celláinak megjelenítése
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

            if (showFullMap) {
                // Ha a teljes térkép látszik, minden cella megjelenik
                if (maze[i][j] == 1) { // Itt a crash bazmeg
                    cell.setFillColor(sf::Color(128, 128, 128)); // Fal
                }
                else {
                    cell.setFillColor(sf::Color::Black); // Üres hely
                }
            }
            else {
                // Fog of war (köd-effekt) alkalmazása
                bool visibleBySurvivors = false;
                for (auto& survivor : survivors) {
                    if (isCellVisible(survivor.position, j, i, SURVIVOR_VIEW_RADIUS, maze)) {
                        visibleBySurvivors = true;
                        break;
                    }
                }
                if (visibleBySurvivors) {
                    if (maze[i][j] == 0) {
                        visibleBySurvivors = true; cell.setFillColor(sf::Color::Black); // Üres hely
                    }
                    else {
                        cell.setFillColor(sf::Color(128, 128, 128)); // Fal
                    }
                }
                else if (isCellInKillerSight(killer, j, i, maze)) {
                    cell.setFillColor(sf::Color(50, 50, 50)); // A gyilkos látókörében
                }
                else {
                    cell.setFillColor(sf::Color(20, 20, 20)); // Köd (fog)
                }
            }

            window.draw(cell); // Cellák kirajzolása
        }
    }
}

void renderMap(sf::RenderWindow& window, const std::vector<std::vector<int>>& maze, const Player& player, const std::vector<Survivor>& survivors, const Killer& killer, bool showFullMap) {
    if (maze.size() < HEIGHT || maze[0].size() < WIDTH) {
        std::cerr << "Error: Maze dimensions are incorrect." << std::endl;
        return;
    }

    sf::Vector2f playerPos = player.position;

    //std::cout << "Rendering map for player at: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;


    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

            if (showFullMap) {
                cell.setFillColor(maze[i][j] == 1 ? sf::Color(128, 128, 128) : sf::Color::Black);
            }
            else {
                if (isCellVisibleWithObstacles(playerPos, j, i, SURVIVOR_VIEW_RADIUS, maze)) {
                    cell.setFillColor(maze[i][j] == 1 ? sf::Color(128, 128, 128) : sf::Color::Black);
                }
                else {
                    cell.setFillColor(sf::Color(20, 20, 20)); // Fog of war
                }
            }

            window.draw(cell);
        }
    }

    // Karakterek renderelése a látómező alapján
    for (const auto& survivor : survivors) {
        if (isCellVisibleWithObstacles(playerPos, survivor.position.x / CELL_SIZE, survivor.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze)) {
            survivor.render(window);
        }
    }

    if (isCellVisibleWithObstacles(playerPos, killer.position.x / CELL_SIZE, killer.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze)) {
        killer.render(window);
    }
}




bool checkCollision(sf::Vector2f position, float playerSize, const std::vector<std::vector<int>>& maze) {
    // Ellenőrizd a maze méretét
    if (maze.empty() || maze[0].empty()) {
        std::cerr << "Error: Maze is empty!" << std::endl;
        return true; // Ha nincs érvényes térkép, számítsd ütközésnek
    }

    // Player méretének figyelembevétele
    float pSize = playerSize - 2.0f;

    // Játékos szélei (left, right, top, bottom)
    float left = position.x;
    float right = position.x + pSize;
    float top = position.y;
    float bottom = position.y + pSize;

    // Cella koordináták kiszámítása
    int leftCell = static_cast<int>(left / CELL_SIZE);
    int rightCell = static_cast<int>(right / CELL_SIZE);
    int topCell = static_cast<int>(top / CELL_SIZE);
    int bottomCell = static_cast<int>(bottom / CELL_SIZE);

    // Határértékek ellenőrzése
    if (leftCell < 0 || rightCell >= static_cast<int>(maze[0].size()) ||
        topCell < 0 || bottomCell >= static_cast<int>(maze.size())) {
        return true; // Ha bármelyik cella kívül esik, ütközésnek számít
    }

    // Ütközések ellenőrzése
    bool collisionDetected = isBlocked(leftCell, topCell, maze) ||
        isBlocked(rightCell, topCell, maze) ||
        isBlocked(leftCell, bottomCell, maze) ||
        isBlocked(rightCell, bottomCell, maze);

    return collisionDetected;
}

// Check if an object can be placed at the given location
bool canPlaceObject(const vector<vector<int>>& maze, int x, int y, int width, int height) {
    for (int i = y; i < y + height; ++i) {
        for (int j = x; j < x + width; ++j) {
            if (i < 0 || j < 0 || i >= HEIGHT || j >= WIDTH) return false; // Out of bounds
            if (maze[i][j] != 0) return false; // Area is already occupied
        }
    }
    return true;
}

// Check if a cell is visible to the player
bool isCellVisible(sf::Vector2f playerPos, int gridX, int gridY, float viewRadius, const vector<vector<int>>& maze) {
    float distance = sqrt(pow(playerPos.x / CELL_SIZE - gridX, 2) + pow(playerPos.y / CELL_SIZE - gridY, 2));
    if (distance > viewRadius) return false;

    int playerGridX = static_cast<int>(playerPos.x / CELL_SIZE);
    int playerGridY = static_cast<int>(playerPos.y / CELL_SIZE);

    int dx = abs(gridX - playerGridX), sx = playerGridX < gridX ? 1 : -1;
    int dy = -abs(gridY - playerGridY), sy = playerGridY < gridY ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        if (playerGridX == gridX && playerGridY == gridY) return true;
        if (maze[playerGridY][playerGridX] != 0) return false;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; playerGridX += sx; }
        if (e2 <= dx) { err += dx; playerGridY += sy; }
    }
}

bool isCellVisibleWithObstacles(const sf::Vector2f& playerPos, int cellX, int cellY, int viewRadius, const std::vector<std::vector<int>>& maze) {
    float dx = cellX - playerPos.x / CELL_SIZE;
    float dy = cellY - playerPos.y / CELL_SIZE;
    float distance = sqrt(dx * dx + dy * dy);

    // Ha a cella túl messze van, nem látható
    if (distance > viewRadius) {
        return false;
    }

    // Raycasting a látóvonal ellenőrzéséhez
    int steps = std::max(abs(static_cast<int>(dx)), abs(static_cast<int>(dy)));
    float stepX = dx / steps;
    float stepY = dy / steps;

    float x = playerPos.x / CELL_SIZE;
    float y = playerPos.y / CELL_SIZE;

    for (int i = 0; i < steps; ++i) {
        x += stepX;
        y += stepY;
        int gridX = static_cast<int>(x);
        int gridY = static_cast<int>(y);

        // Ha fal van az útban, a falat láthatóvá tesszük, de mögötte lévő cellákat nem
        if (maze[gridY][gridX] == 1) {
            return gridX == cellX && gridY == cellY;
        }
    }

    // Ha nincs fal az útban, a cella látható
    return true;
}

bool isCellInKillerSight(const Player& killer, int gridX, int gridY, const std::vector<std::vector<int>>& maze) {
    float dx = gridX - (killer.position.x / CELL_SIZE);
    float dy = gridY - (killer.position.y / CELL_SIZE);
    float dotProduct = dx * killer.lastDirection.x + dy * killer.lastDirection.y;

    // Csak akkor, ha a killer előrefelé néz
    if (dotProduct <= 0) return false;

    // Ha a cella látható a killer számára
    return isCellVisible(killer.position, gridX, gridY, KILLER_VIEW_DISTANCE, maze);
}

#include <random>


// Returns the distance to the player closest to thisPlayer
float minDistanceFromOtherPlayers(const std::vector<sf::Vector2f>& players, sf::Vector2f thisPlayer) {
    if (players.empty()) {
        return MIN_SPAWN_PLAYER_DISTANCE;
    }

    float minDist = WIDTH * HEIGHT * CELL_SIZE; //The maximum possible distance is the area of the map

    for (const auto& otherPlayer : players) {
        float dist = std::sqrt( std::pow(thisPlayer.x - otherPlayer.x, 2) + std::pow(thisPlayer.y - otherPlayer.y, 2));
        minDist = std::min(minDist, dist);
    }
    return minDist;
}




// Random pozíció generálása
sf::Vector2f generateRandomPosition(const std::vector<std::vector<int>>& maze, int cellWidth, int cellHeight, const std::vector<sf::Vector2f> &playerPositions) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(0, WIDTH - cellWidth);
    std::uniform_int_distribution<> distY(0, HEIGHT - cellHeight);

    int x, y;
    sf::Vector2f playerPosOnMap;

    do {
        x = distX(gen);
        y = distY(gen);
        playerPosOnMap = sf::Vector2f( x * CELL_SIZE, y * CELL_SIZE );
    } while (!canPlaceObject(maze, x, y, cellWidth, cellHeight) && 
        minDistanceFromOtherPlayers(playerPositions,playerPosOnMap) >= MIN_SPAWN_PLAYER_DISTANCE);

    return playerPosOnMap;
}


