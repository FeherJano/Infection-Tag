#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include "structures.hpp" // Predefined object structures

using namespace std;

const int WIDTH = 90;  // Map width
const int HEIGHT = 90; // Map height
const int CELL_SIZE = 10; // Cell size in the display
const int GRID_ROWS = 3; // Number of blocks in rows
const int GRID_COLS = 3; // Number of blocks in columns

const float SURVIVOR_VIEW_RADIUS = 20.0f; // Survivor vision radius (in cells)
const float KILLER_VIEW_DISTANCE = 25.0f; // Killer's sight distance (in cells)
const float SURVIVOR_MOVE_SPEED = 100.0f; // Survivor speed
const float KILLER_MOVE_SPEED = 120.0f; // Killer speed
const float KILLER_HIT_COOLDOWN = 1.0f; // Killer attack cooldown
const float SURVIVOR_HIT_SPEED_BOOST = 100.0f; // Speed boost for survivors when hit
const int HEALTH_STATE_COUNT = 3; // Number of health states

enum HealthState { HEALTHY, INJURED, DYING, DEAD };

// Checks if a cell is blocked (a wall or obstacle)
bool isBlocked(int gridX, int gridY, const vector<vector<int>>& maze) {
    if (gridX < 0 || gridX >= WIDTH || gridY < 0 || gridY >= HEIGHT) {
        return true;  // Treat out-of-bounds as blocked
    }
    return maze[gridY][gridX] != 0; // Cell is blocked if it's not empty (0)
}

// Updated collision detection for more precise wall sensing
bool checkCollision(sf::Vector2f position, float playerSize, const vector<vector<int>>& maze) {
    // Take player's size into account (pSize = half of the character's width/height)
    float pSize = playerSize - 2.0f;  // Player "radius"

    // Player's edges (left, right, top, bottom)
    float left = position.x;
    float right = position.x + pSize;
    float top = position.y;
    float bottom = position.y + pSize;

    // Check for walls around the player
    bool collisionDetected = isBlocked(static_cast<int>(left / CELL_SIZE), static_cast<int>(top / CELL_SIZE), maze) ||
        isBlocked(static_cast<int>(right / CELL_SIZE), static_cast<int>(top / CELL_SIZE), maze) ||
        isBlocked(static_cast<int>(left / CELL_SIZE), static_cast<int>(bottom / CELL_SIZE), maze) ||
        isBlocked(static_cast<int>(right / CELL_SIZE), static_cast<int>(bottom / CELL_SIZE), maze);

    return collisionDetected;
}

class Player {
public:
    sf::Vector2f position; // Player position
    sf::Vector2f lastDirection; // Last movement direction (for the killer)
    float moveSpeed; // Player speed
    float hitCooldownTimer = 0.0f; // Killer attack cooldown

    Player(float startX, float startY, float speed) : position(startX, startY), lastDirection(1.0f, 0.0f), moveSpeed(speed) {}

    // Player movement with collision detection
    void move(float dirX, float dirY, float deltaTime, const vector<vector<int>>& maze) {
        float length = sqrt(dirX * dirX + dirY * dirY);
        if (length != 0) {
            dirX /= length;
            dirY /= length;
        }

        sf::Vector2f offset(dirX * moveSpeed * deltaTime, dirY * moveSpeed * deltaTime);

        // Check X-axis movement
        sf::Vector2f newXPos = position + sf::Vector2f(offset.x, 0);
        if (!checkCollision(newXPos, CELL_SIZE, maze)) {
            position.x += offset.x;
        }

        // Check Y-axis movement
        sf::Vector2f newYPos = position + sf::Vector2f(0, offset.y);
        if (!checkCollision(newYPos, CELL_SIZE, maze)) {
            position.y += offset.y;
        }

        // Update the last movement direction
        if (dirX != 0 || dirY != 0) {
            lastDirection = sf::Vector2f(dirX, dirY);
        }
    }

    virtual void update(float deltaTime) {
        if (hitCooldownTimer > 0) {
            hitCooldownTimer -= deltaTime;
        }
    }
};

// Survivor class derived from Player
class Survivor : public Player {
public:
    HealthState healthState;
    float speedBoostTimer = 0.0f;
    float dyingTimer = 0.0f;  // Timer for the DYING state
    const float maxDyingTime = 30.0f;  // Max time for the DYING state

    Survivor(float startX, float startY) : Player(startX, startY, SURVIVOR_MOVE_SPEED), healthState(HEALTHY), dyingTimer(0.0f) {}

    // Survivor gets hit and changes health state
    void getHit() {
        if (healthState == HEALTHY) {
            healthState = INJURED;
            speedBoostTimer = 3.0f;  // Speed boost for 3 seconds
        }
        else if (healthState == INJURED) {
            healthState = DYING;
            dyingTimer = maxDyingTime;  // Set dying timer
        }
    }

    void update(float deltaTime) override {
        Player::update(deltaTime);

        if (healthState == DEAD) {
            moveSpeed = 0;  // Can't move
            return;
        }

        if (healthState == DYING) {
            moveSpeed = 0;  // Can't move in DYING state

            dyingTimer -= deltaTime;  // Reduce timer

            if (dyingTimer <= 0) {
                healthState = DEAD;  // Change to DEAD state
            }
            return;
        }

        // Apply speed boost if active
        if (speedBoostTimer > 0) {
            speedBoostTimer -= deltaTime;
            moveSpeed = SURVIVOR_MOVE_SPEED + SURVIVOR_HIT_SPEED_BOOST;
        }
        else {
            moveSpeed = SURVIVOR_MOVE_SPEED;
        }
    }

    // Get progress of DYING state (for visual representation)
    float getDyingProgress() const {
        if (healthState == DYING) {
            return 1.0f - (dyingTimer / maxDyingTime);  // Progress ratio for dying state
        }
        return 0.0f;
    }

    // Render the survivor on screen based on health state
    void render(sf::RenderWindow& window) {
        sf::RectangleShape survivorShape(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        survivorShape.setPosition(position);

        if (healthState == HEALTHY) {
            survivorShape.setFillColor(sf::Color::Green); // Fully green
            window.draw(survivorShape);
        }
        else if (healthState == INJURED) {
            survivorShape.setFillColor(sf::Color::Transparent); // Transparent center
            survivorShape.setOutlineThickness(2);
            survivorShape.setOutlineColor(sf::Color::Green); // Green outline

            // Small green square in the middle
            sf::RectangleShape smallSquare(sf::Vector2f(CELL_SIZE / 2 + 1, CELL_SIZE / 2 + 1));
            smallSquare.setPosition(position.x + CELL_SIZE / 4, position.y + CELL_SIZE / 4); // Centered
            smallSquare.setFillColor(sf::Color(128, 128, 128));
            window.draw(smallSquare);
            window.draw(survivorShape);
        }
        else if (healthState == DYING) {
            survivorShape.setFillColor(sf::Color::Transparent); // Transparent
            survivorShape.setOutlineThickness(2);
            survivorShape.setOutlineColor(sf::Color::Green); // Green outline
            window.draw(survivorShape); // Draw green outline

            // Draw the gray progress bar (based on the timer)
            float progress = 1.0f - (dyingTimer / maxDyingTime);  // Progress ratio

            // Gradual gray outline, drawn exactly over the green outline
            if (progress > 0.0f) {
                // Top edge left to right
                float partialWidth = std::min(progress * 5.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialTop(sf::Vector2f(partialWidth, 2)); // Thin outline
                partialTop.setPosition(position.x - 2, position.y - 2); // Top part
                partialTop.setFillColor(sf::Color(128, 128, 128)); // Gray
                window.draw(partialTop);
            }

            if (progress > 0.25f) {
                // Right side, top to bottom
                float partialHeight = std::min((progress - 0.25f) * 5.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialRight(sf::Vector2f(2, partialHeight)); // Thin outline
                partialRight.setPosition(position.x + CELL_SIZE, position.y - 2); // Right side
                partialRight.setFillColor(sf::Color(128, 128, 128)); // Gray
                window.draw(partialRight);
            }

            if (progress > 0.5f) {
                // Bottom edge, right to left
                float partialWidth = std::min((progress - 0.5f) * 6.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialBottom(sf::Vector2f(partialWidth, 2)); // Thin outline
                partialBottom.setPosition(position.x + CELL_SIZE - partialWidth + 2, position.y + CELL_SIZE); // Bottom part
                partialBottom.setFillColor(sf::Color(128, 128, 128)); // Gray
                window.draw(partialBottom);
            }

            if (progress > 0.75f) {
                // Left side, bottom to top
                float partialHeight = std::min((progress - 0.75f) * 5.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialLeft(sf::Vector2f(2, partialHeight)); // Thin outline
                partialLeft.setPosition(position.x - 2, position.y + CELL_SIZE - partialHeight); // Left side
                partialLeft.setFillColor(sf::Color(128, 128, 128)); // Gray
                window.draw(partialLeft);
            }
        }
        else if (healthState == DEAD) {
            survivorShape.setFillColor(sf::Color::Transparent); // Transparent
            survivorShape.setOutlineThickness(2);
            survivorShape.setOutlineColor(sf::Color(180, 180, 180)); // Gray outline in DEAD state
            window.draw(survivorShape);
        }
    }
};

// Killer class derived from Player
class Killer : public Player {
public:
    Killer(float startX, float startY) : Player(startX, startY, KILLER_MOVE_SPEED) {}

    bool canHit() {
        return hitCooldownTimer <= 0.0f;
    }

    void hit(Survivor& survivor) {
        if (canHit()) {
            // Check that survivor is not in DYING or DEAD state
            if (survivor.healthState != DYING && survivor.healthState != DEAD) {
                survivor.getHit(); // Only hit if the survivor is not DYING or DEAD
                hitCooldownTimer = KILLER_HIT_COOLDOWN; // Set 1-second cooldown
                moveSpeed = KILLER_MOVE_SPEED * 0.3f; // Reduce speed after hit
            }
        }
    }

    void update(float deltaTime) override {
        Player::update(deltaTime);

        if (hitCooldownTimer <= 0) {
            moveSpeed = KILLER_MOVE_SPEED; // Reset speed after cooldown
        }
    }
};

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

// Place objects on the map
void placeObjects(vector<vector<int>>& maze) {
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

// Check if the cell is within the killer's line of sight
bool isCellInKillerSight(Player& killer, int gridX, int gridY, const vector<vector<int>>& maze) {
    float dx = gridX - (killer.position.x / CELL_SIZE);
    float dy = gridY - (killer.position.y / CELL_SIZE);
    float dotProduct = dx * killer.lastDirection.x + dy * killer.lastDirection.y;
    if (dotProduct <= 0) return false;

    return isCellVisible(killer.position, gridX, gridY, KILLER_VIEW_DISTANCE, maze);
}

// Check if the killer and survivor collide
bool checkCollisionBetween(Killer& killer, Survivor& survivor) {
    if (survivor.healthState == DYING || survivor.healthState == DEAD) {
        return false; // No collision for DYING or DEAD survivors
    }

    float distance = sqrt(pow(killer.position.x - survivor.position.x, 2) + pow(killer.position.y - survivor.position.y, 2));
    return distance < CELL_SIZE; // Killer is close enough to the survivor
}

class Task {
public:
    sf::Vector2f position;
    float progress = 0.0f;
    const float maxProgress = 30.0f; // 30 seconds to complete

    Task(float startX, float startY) {
        position = sf::Vector2f(startX, startY);
    }

    // Update task progress
    void update(float deltaTime, int numSurvivorsOnTask) {
        if (numSurvivorsOnTask > 0) {
            progress += deltaTime * numSurvivorsOnTask; // More survivors speed up the task
            if (progress > maxProgress) {
                progress = maxProgress;
            }
        }
    }

    // Render the task on the map
    void render(sf::RenderWindow& window, bool isVisible, bool showFullMap) {
        if (!isVisible && !showFullMap) {
            return; // Don't render if not visible and not in full map view
        }

        sf::RectangleShape taskShape(sf::Vector2f(CELL_SIZE * 3, CELL_SIZE * 3));
        taskShape.setPosition(position);
        taskShape.setFillColor(sf::Color(150, 0, 150)); // Dark purple

        float progressRatio = progress / maxProgress;
        sf::RectangleShape progressShape(sf::Vector2f(CELL_SIZE * 3 * progressRatio, CELL_SIZE * 3));
        progressShape.setPosition(position);
        progressShape.setFillColor(sf::Color(200, 0, 200)); // Light purple

        window.draw(taskShape);
        window.draw(progressShape);

        if (progress >= maxProgress) {
            taskShape.setOutlineThickness(2);
            taskShape.setOutlineColor(sf::Color::White);
            window.draw(taskShape);
        }
    }
};

// Place tasks on the map
void placeTasks(vector<Task>& tasks, vector<vector<int>>& maze, int numSurvivors) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> blockDistX(0, GRID_COLS - 1);
    uniform_int_distribution<> blockDistY(0, GRID_ROWS - 1);

    // Set the number of tasks (survivor count + 3)
    int numTasks = numSurvivors + 3;

    // Generate tasks
    for (int i = 0; i < numTasks; ++i) {
        int blockX, blockY;
        do {
            blockX = blockDistX(gen) * (WIDTH / GRID_COLS);
            blockY = blockDistY(gen) * (HEIGHT / GRID_ROWS);
        } while (!canPlaceObject(maze, blockX, blockY, 3, 3)); // Ensure area is free

        // Add task
        tasks.emplace_back(blockX * CELL_SIZE, blockY * CELL_SIZE);
    }
}

// Check if a survivor can work on the task (only if HEALTHY or INJURED and within the task area)
bool isSurvivorOnTask(const Survivor& survivor, const Task& task) {
    return (survivor.healthState == HEALTHY || survivor.healthState == INJURED) &&
        (survivor.position.x >= task.position.x && survivor.position.x <= task.position.x + CELL_SIZE * 3 &&
            survivor.position.y >= task.position.y && survivor.position.y <= task.position.y + CELL_SIZE * 3);
}

// Check if a survivor can see the task (if it's in the survivor's vision)
bool isTaskVisibleToSurvivor(const Task& task, const Survivor& survivor, const vector<vector<int>>& maze) {
    return isCellVisible(survivor.position, task.position.x / CELL_SIZE, task.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze);
}


int main() {
    srand(time(0));

    // 2D map, initially empty (0 = empty space, 1 = wall)
    vector<vector<int>> maze(HEIGHT, vector<int>(WIDTH, 0));

    // Generate map with objects and buildings
    placeObjects(maze);

    // Add tasks
    vector<Task> tasks;
    int numSurvivors = 1; // Example with 1 survivor
    placeTasks(tasks, maze, numSurvivors);

    // Add players
    Killer killer(1.0f * CELL_SIZE, 1.0f * CELL_SIZE);
    Survivor survivor((WIDTH - 2.0f) * CELL_SIZE, (HEIGHT - 2.0f) * CELL_SIZE);

    sf::RenderWindow window(sf::VideoMode(WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE), "Asymmetric Multiplayer Game");

    sf::Clock clock;

    bool showFullMap = false;  // Fog of war state

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle "M" key press
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M) {
                showFullMap = !showFullMap;  // Toggle fog of war
            }

            // Handle "H" key press
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H) {
                survivor.healthState = HEALTHY;
            }

            // Handle "Q" key press
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q) {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        float dirX = 0.0f, dirY = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dirY -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dirY += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dirX -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dirX += 1.0f;

        killer.move(dirX, dirY, deltaTime, maze);

        dirX = 0.0f;
        dirY = 0.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) dirY -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) dirY += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) dirX -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dirX += 1.0f;

        survivor.move(dirX, dirY, deltaTime, maze);

        // Update tasks and progress
        for (Task& task : tasks) {
            int numSurvivorsOnTask = isSurvivorOnTask(survivor, task) ? 1 : 0;
            task.update(deltaTime, numSurvivorsOnTask);
        }

        // Update players
        killer.update(deltaTime);
        survivor.update(deltaTime);

        // Check collision between killer and survivor
        if (checkCollisionBetween(killer, survivor) && killer.canHit()) {
            killer.hit(survivor); // Hit survivor if not in DYING or DEAD state
        }

        window.clear(sf::Color::Black);

        // Render the map
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

                if (showFullMap) {
                    if (maze[i][j] == 1) {
                        cell.setFillColor(sf::Color(128, 128, 128)); // Wall
                    }
                    else {
                        cell.setFillColor(sf::Color::Black); // Empty space
                    }
                }
                else {
                    // Apply fog of war
                    if (isCellVisible(survivor.position, j, i, SURVIVOR_VIEW_RADIUS, maze)) {
                        if (maze[i][j] == 1) {
                            cell.setFillColor(sf::Color(128, 128, 128)); // Wall
                        }
                        else {
                            cell.setFillColor(sf::Color::Black); // Empty space
                        }
                    }
                    else if (isCellInKillerSight(killer, j, i, maze)) {
                        cell.setFillColor(sf::Color(50, 50, 50)); // In killer's sight
                    }
                    else {
                        cell.setFillColor(sf::Color(20, 20, 20)); // Fog
                    }
                }

                window.draw(cell);
            }
        }

        // Render tasks (only if visible to survivor or full map view)
        for (Task& task : tasks) {
            bool isVisible = isTaskVisibleToSurvivor(task, survivor, maze);
            task.render(window, isVisible, showFullMap);
        }

        // Render killer
        sf::RectangleShape killerRect(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        killerRect.setPosition(killer.position.x, killer.position.y);
        killerRect.setFillColor(sf::Color::Red);
        window.draw(killerRect);

        // Render survivor based on health state
        survivor.render(window);

        window.display();
    }

    return 0;
}
