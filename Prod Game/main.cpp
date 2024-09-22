#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include "structures.hpp" // Az előre definiált objektumstruktúrák

using namespace std;

const int WIDTH = 90;  // A pálya szélessége
const int HEIGHT = 90; // A pálya magassága
const int CELL_SIZE = 10; // A cellák mérete a megjelenítésben
const int GRID_ROWS = 3; // Blokkok száma sorokban
const int GRID_COLS = 3; // Blokkok száma oszlopokban

const float SURVIVOR_VIEW_RADIUS = 20.0f; // Túlélő látóköre (cellákban)
const float KILLER_VIEW_DISTANCE = 25.0f; // Gyilkos látótávolsága (cellákban)
const float SURVIVOR_MOVE_SPEED = 100.0f; // Túlélő sebessége
const float KILLER_MOVE_SPEED = 120.0f; // Gyilkos sebessége
const float KILLER_HIT_COOLDOWN = 1.0f; // Gyilkos támadási időkorlát
const float SURVIVOR_HIT_SPEED_BOOST = 100.0f; // Túlélő sebességnövekedése eltaláláskor
const int HEALTH_STATE_COUNT = 3; // Az egészség állapotainak száma

enum HealthState { HEALTHY, INJURED, DYING, DEAD };

// Ellenőrizzük, hogy egy adott cellában van-e fal vagy akadály
bool isBlocked(int gridX, int gridY, const vector<vector<int>>& maze) {
    if (gridX < 0 || gridX >= WIDTH || gridY < 0 || gridY >= HEIGHT) {
        return true;  // Pályán kívül blokkoltnak tekintjük
    }
    return maze[gridY][gridX] != 0; // A cella blokkolt, ha nem 0 (nem üres)
}

// Frissített ütközésellenőrzés a pontosabb falérzékeléshez
bool checkCollision(sf::Vector2f position, float playerSize, const vector<vector<int>>& maze) {
    // A játékos méretét vesszük figyelembe (pSize = a karakter fél szélessége/magassága)
    float pSize = playerSize - 2.0f;  // A játékos „sugara”

    // Játékos szélei (bal, jobb, fent, lent)
    float left = position.x;
    float right = position.x + pSize;
    float top = position.y;
    float bottom = position.y + pSize;

    // Ellenőrizzük, hogy a játékos bármelyik oldalán van-e fal
    bool collisionDetected = isBlocked(static_cast<int>(left / CELL_SIZE), static_cast<int>(top / CELL_SIZE), maze) ||
        isBlocked(static_cast<int>(right / CELL_SIZE), static_cast<int>(top / CELL_SIZE), maze) ||
        isBlocked(static_cast<int>(left / CELL_SIZE), static_cast<int>(bottom / CELL_SIZE), maze) ||
        isBlocked(static_cast<int>(right / CELL_SIZE), static_cast<int>(bottom / CELL_SIZE), maze);

    // Ha van ütközés, akkor visszatérünk
    return collisionDetected;
}

class Player {
public:
    sf::Vector2f position; // Játékos pozíciója lebegőpontos formában
    sf::Vector2f lastDirection; // Utolsó mozgásirány (csak a gyilkosnak kell)
    float moveSpeed; // Játékos sebessége
    float hitCooldownTimer = 0.0f; // Gyilkos támadási időkorlát

    Player(float startX, float startY, float speed) : position(startX, startY), lastDirection(1.0f, 0.0f), moveSpeed(speed) {}

    // Játékos mozgatása folyamatosan, ütközésellenőrzéssel
    void move(float dirX, float dirY, float deltaTime, const vector<vector<int>>& maze) {
        float length = sqrt(dirX * dirX + dirY * dirY);
        if (length != 0) {
            dirX /= length;
            dirY /= length;
        }

        sf::Vector2f offset(dirX * moveSpeed * deltaTime, dirY * moveSpeed * deltaTime);

        // Először az X irányú mozgás ellenőrzése
        sf::Vector2f newXPos = position + sf::Vector2f(offset.x, 0);
        if (!checkCollision(newXPos, CELL_SIZE, maze)) {
            position.x += offset.x;
        }

        // Majd az Y irányú mozgás ellenőrzése
        sf::Vector2f newYPos = position + sf::Vector2f(0, offset.y);
        if (!checkCollision(newYPos, CELL_SIZE, maze)) {
            position.y += offset.y;
        }

        // Frissítjük az utolsó mozgásirányt
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

// Túlélő osztály, amely a Player osztályból származik
class Survivor : public Player {
public:
    HealthState healthState;
    float speedBoostTimer = 0.0f;
    float dyingTimer = 0.0f;  // Timer, ami a Dying állapot idejét követi
    const float maxDyingTime = 30.0f;  // Maximum idő, ameddig a Survivor Dying állapotban lehet

    Survivor(float startX, float startY) : Player(startX, startY, SURVIVOR_MOVE_SPEED), healthState(HEALTHY), dyingTimer(0.0f) {}

    void getHit() {
        if (healthState == HEALTHY) {
            healthState = INJURED;
            speedBoostTimer = 3.0f;  // 3 másodperc sebességnövekedés
        }
        else if (healthState == INJURED) {
            healthState = DYING;
            dyingTimer = maxDyingTime;  // Beállítjuk a timer-t Dying állapotban
        }
    }

    void update(float deltaTime) override {
        Player::update(deltaTime);

        if (healthState == DEAD) {
            moveSpeed = 0;  // Nem tud mozogni
            return;
        }

        if (healthState == DYING) {
            moveSpeed = 0;  // Dying állapotban nem tud mozogni

            // Timer csökkentése
            dyingTimer -= deltaTime;

            // Ha a timer lejár, átlép Dead állapotba
            if (dyingTimer <= 0) {
                healthState = DEAD;
            }

            return;
        }

        // Ha van sebességnövekedés, alkalmazzuk
        if (speedBoostTimer > 0) {
            speedBoostTimer -= deltaTime;
            moveSpeed = SURVIVOR_MOVE_SPEED + SURVIVOR_HIT_SPEED_BOOST;
        }
        else {
            moveSpeed = SURVIVOR_MOVE_SPEED;
        }
    }

    // Vizuális timer progresszió megjelenítése (a körkörös szürke keret)
    float getDyingProgress() const {
        if (healthState == DYING) {
            return 1.0f - (dyingTimer / maxDyingTime);  // A progressz aránya a szürke kerethez
        }
        return 0.0f;
    }


    void render(sf::RenderWindow& window) {
        sf::RectangleShape survivorShape(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        survivorShape.setPosition(position);

        

        if (healthState == HEALTHY) {
            survivorShape.setFillColor(sf::Color::Green); // Teljesen zöld
            window.draw(survivorShape);
        }
        else if (healthState == INJURED) {
            survivorShape.setFillColor(sf::Color::Transparent); // Átlátszó közép
            survivorShape.setOutlineThickness(2);
            survivorShape.setOutlineColor(sf::Color::Green); // Zöld keret

            // Kis zöld négyzet középen
            sf::RectangleShape smallSquare(sf::Vector2f(CELL_SIZE / 2 + 1 , CELL_SIZE / 2 + 1));
            smallSquare.setPosition(position.x + CELL_SIZE / 4, position.y + CELL_SIZE / 4); // Középre igazítva
            smallSquare.setFillColor(sf::Color(128, 128, 128));
            window.draw(smallSquare);
            window.draw(survivorShape);
        }
        else if (healthState == DYING) {
            survivorShape.setFillColor(sf::Color::Transparent); // Átlátszó
            survivorShape.setOutlineThickness(2);
            survivorShape.setOutlineColor(sf::Color::Green); // Csak zöld keret
            window.draw(survivorShape); // Zöld keret megjelenítése

            // Fokozatos szürke keret rajzolása (timer alapján)
            float progress = 1.0f - (dyingTimer / maxDyingTime);  // 0.0 és 1.0 között

            // Fokozatos szürke keret rajzolása, pontosan a zöld keret helyén
            if (progress > 0.0f) {
                // Felső keret balról jobbra
                float partialWidth = std::min(progress * 5.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialTop(sf::Vector2f(partialWidth, 2)); // Keskeny keret
                partialTop.setPosition(position.x - 2, position.y - 2); // Felső rész
                partialTop.setFillColor(sf::Color(128, 128, 128)); // Szürke keret
                window.draw(partialTop);
            }

            if (progress > 0.25f) {
                // Jobb oldal keret fentről lefelé
                float partialHeight = std::min((progress - 0.25f) * 5.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialRight(sf::Vector2f(2, partialHeight)); // Keskeny keret
                partialRight.setPosition(position.x + CELL_SIZE, position.y - 2); // Jobb oldal
                partialRight.setFillColor(sf::Color(128, 128, 128)); // Szürke keret
                window.draw(partialRight);
            }

            if (progress > 0.5f) {
                // Alsó keret jobbról balra
                float partialWidth = std::min((progress - 0.5f) * 6.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialBottom(sf::Vector2f(partialWidth, 2)); // Keskeny keret
                partialBottom.setPosition(position.x + CELL_SIZE - partialWidth + 2, position.y + CELL_SIZE); // Alsó rész
                partialBottom.setFillColor(sf::Color(128, 128, 128)); // Szürke keret
                window.draw(partialBottom);
            }

            if (progress > 0.75f) {
                // Bal oldal keret lentről felfelé
                float partialHeight = std::min((progress - 0.75f) * 5.0f * CELL_SIZE, static_cast<float>(CELL_SIZE) + 4);
                sf::RectangleShape partialLeft(sf::Vector2f(2, partialHeight)); // Keskeny keret
                partialLeft.setPosition(position.x - 2, position.y + CELL_SIZE - partialHeight); // Bal oldal
                partialLeft.setFillColor(sf::Color(128, 128, 128)); // Szürke keret
                window.draw(partialLeft);
            }
        }
        else if (healthState == DEAD) {
            survivorShape.setFillColor(sf::Color::Transparent); // Átlátszó
            survivorShape.setOutlineThickness(2);
            survivorShape.setOutlineColor(sf::Color(180, 180, 180)); // Szürke keret a DEAD állapotban
            window.draw(survivorShape);
        }

        
    }
};

// Gyilkos osztály, amely a Player osztályból származik
class Killer : public Player {
public:
    Killer(float startX, float startY) : Player(startX, startY, KILLER_MOVE_SPEED) {}

    bool canHit() {
        return hitCooldownTimer <= 0.0f;
    }

    void hit(Survivor& survivor) {
        if (canHit()) {
            // Ellenőrizzük, hogy a survivor nem "DYING" vagy "DEAD" állapotban van
            if (survivor.healthState != DYING && survivor.healthState != DEAD) {
                survivor.getHit(); // Csak akkor üti meg, ha nem DYING vagy DEAD
                hitCooldownTimer = KILLER_HIT_COOLDOWN; // 1 másodperces időkorlát
                moveSpeed = KILLER_MOVE_SPEED * 0.3f; // Sebesség csökkentése
            }
        }
    }


    void update(float deltaTime) override {
        Player::update(deltaTime);

        if (hitCooldownTimer <= 0) {
            moveSpeed = KILLER_MOVE_SPEED; // Visszaállítjuk a normál sebességet
        }
    }
};

// Ütközésellenőrzés (objektumok ne lógjanak egymásba)
bool canPlaceObject(const vector<vector<int>>& maze, int x, int y, int width, int height) {
    for (int i = y; i < y + height; ++i) {
        for (int j = x; j < x + width; ++j) {
            if (i < 0 || j < 0 || i >= HEIGHT || j >= WIDTH) return false; // Kilóg a pálya szélén
            if (maze[i][j] != 0) return false; // Már van valami itt
        }
    }
    return true;
}

// Objektumok elhelyezése a blokkok szerint
void placeObjects(vector<vector<int>>& maze) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> smallObjDist(0, SMALL_OBJECTS.size() - 1);
    uniform_int_distribution<> largeObjDist(0, LARGE_OBJECTS.size() - 1);
    uniform_int_distribution<> mainBuildingDist(0, MAIN_BUILDINGS.size() - 1);
    uniform_int_distribution<> rotDist(0, 3); // 0-3 közötti forgatás
    uniform_int_distribution<> blockDistX(0, GRID_COLS - 1);
    uniform_int_distribution<> blockDistY(0, GRID_ROWS - 1);

    // Blokk mérete sorokban és oszlopokban
    int blockWidth = WIDTH / GRID_COLS;
    int blockHeight = HEIGHT / GRID_ROWS;

    // Blokkok listája, hogy figyeljük, hol helyeztünk el objektumokat
    vector<pair<int, int>> availableBlocks;

    for (int i = 0; i < GRID_ROWS; ++i) {
        for (int j = 0; j < GRID_COLS; ++j) {
            availableBlocks.push_back(make_pair(i, j));
        }
    }

    // Főépület elhelyezése véletlenszerű blokkban
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

    // Közepes objektumok elhelyezése minden blokkban
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

    // Kis objektumok elhelyezése a maradék helyekre
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

bool isCellInKillerSight(Player& killer, int gridX, int gridY, const vector<vector<int>>& maze) {
    float dx = gridX - (killer.position.x / CELL_SIZE);
    float dy = gridY - (killer.position.y / CELL_SIZE);
    float dotProduct = dx * killer.lastDirection.x + dy * killer.lastDirection.y;
    if (dotProduct <= 0) return false;

    return isCellVisible(killer.position, gridX, gridY, KILLER_VIEW_DISTANCE, maze);
}


bool checkCollisionBetween(Killer& killer, Survivor& survivor) {
    if (survivor.healthState == DYING || survivor.healthState == DEAD) {
        return false; // Dying vagy Dead survivor-ök esetében nincs ütközés, amit ütésnek számítunk
    }

    float distance = sqrt(pow(killer.position.x - survivor.position.x, 2) + pow(killer.position.y - survivor.position.y, 2));
    return distance < CELL_SIZE; // Ha a gyilkos közel van a túlélőhöz
}

class Task {
public:
    sf::Vector2f position;
    float progress = 0.0f;
    const float maxProgress = 30.0f; // 30 másodperc a teljesítéshez

    Task(float startX, float startY) {
        position = sf::Vector2f(startX, startY);
    }

    // Frissítjük a task progresszét
    void update(float deltaTime, int numSurvivorsOnTask) {
        if (numSurvivorsOnTask > 0) {
            progress += deltaTime * numSurvivorsOnTask; // Több survivor gyorsítja a taskot
            if (progress > maxProgress) {
                progress = maxProgress;
            }
        }
    }

    // Megjelenítjük a taskot a pályán
    void render(sf::RenderWindow& window, bool isVisible, bool showFullMap) {
        if (!isVisible && !showFullMap) {
            return; // Ha nincs a survivor látómezejében és a teljes pálya nincs megjelenítve, nem rajzoljuk
        }

        sf::RectangleShape taskShape(sf::Vector2f(CELL_SIZE * 3, CELL_SIZE * 3));
        taskShape.setPosition(position);
        taskShape.setFillColor(sf::Color(150, 0, 150)); // Sötét lila alap

        float progressRatio = progress / maxProgress;
        sf::RectangleShape progressShape(sf::Vector2f(CELL_SIZE * 3 * progressRatio, CELL_SIZE * 3));
        progressShape.setPosition(position);
        progressShape.setFillColor(sf::Color(200, 0, 200)); // Világosabb lila

        window.draw(taskShape);
        window.draw(progressShape);

        if (progress >= maxProgress) {
            taskShape.setOutlineThickness(2);
            taskShape.setOutlineColor(sf::Color::White);
            window.draw(taskShape);
        }
    }
};

// Taskok elhelyezése a pályán
void placeTasks(vector<Task>& tasks, vector<vector<int>>& maze, int numSurvivors) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> blockDistX(0, GRID_COLS - 1);
    uniform_int_distribution<> blockDistY(0, GRID_ROWS - 1);

    // Meghatározzuk a taskok számát (túlélők száma + 3)
    int numTasks = numSurvivors + 3;

    // Taskok generálása
    for (int i = 0; i < numTasks; ++i) {
        int blockX, blockY;
        do {
            blockX = blockDistX(gen) * (WIDTH / GRID_COLS);
            blockY = blockDistY(gen) * (HEIGHT / GRID_ROWS);
        } while (!canPlaceObject(maze, blockX, blockY, 3, 3)); // Biztosítjuk, hogy a terület szabad

        // Task hozzáadása
        tasks.emplace_back(blockX * CELL_SIZE, blockY * CELL_SIZE);
    }
}

// Ellenőrizzük, hogy a survivor dolgozhat-e a taskon (csak HEALTHY vagy INJURED állapotban és a task területén van-e)
bool isSurvivorOnTask(const Survivor& survivor, const Task& task) {
    return (survivor.healthState == HEALTHY || survivor.healthState == INJURED) &&
        (survivor.position.x >= task.position.x && survivor.position.x <= task.position.x + CELL_SIZE * 3 &&
            survivor.position.y >= task.position.y && survivor.position.y <= task.position.y + CELL_SIZE * 3);
}


// Ellenőrizzük, hogy a survivor látja-e a taskot (ha a task a látómezejében van)
bool isTaskVisibleToSurvivor(const Task& task, const Survivor& survivor, const vector<vector<int>>& maze) {
    return isCellVisible(survivor.position, task.position.x / CELL_SIZE, task.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze);
}


int main() {
    srand(time(0));

    // 2D pálya, kezdetben üres (0 = üres hely, 1 = fal)
    vector<vector<int>> maze(HEIGHT, vector<int>(WIDTH, 0));

    // Pálya generálása objektumokkal és házakkal
    placeObjects(maze);

    // Taskok hozzáadása
    vector<Task> tasks;
    int numSurvivors = 1; // Ez a példa egy túlélőt tartalmaz
    placeTasks(tasks, maze, numSurvivors);

    // Játékosok hozzáadása
    Killer killer(1.0f * CELL_SIZE, 1.0f * CELL_SIZE);
    Survivor survivor((WIDTH - 2.0f) * CELL_SIZE, (HEIGHT - 2.0f) * CELL_SIZE);

    sf::RenderWindow window(sf::VideoMode(WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE), "Asymmetric Multiplayer Game");

    sf::Clock clock;

    bool showFullMap = false;  // Fog of war állapota

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // "M" billentyű lekezelése
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M) {
                showFullMap = !showFullMap;  // Váltás a fog of war állapota között
            }

            // "H" billentyű lekezelése
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::H) {
                survivor.healthState = HEALTHY;
            }

            // "Q" billentyű lekezelése
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

        // Taskok frissítése és progress növelése
        for (Task& task : tasks) {
            int numSurvivorsOnTask = isSurvivorOnTask(survivor, task) ? 1 : 0;
            task.update(deltaTime, numSurvivorsOnTask);
        }

        // Frissítések
        killer.update(deltaTime);
        survivor.update(deltaTime);

        // Gyilkos túlélő ütközésvizsgálat
        if (checkCollisionBetween(killer, survivor) && killer.canHit()) {
            killer.hit(survivor); // Csak akkor üti meg, ha nem DYING vagy DEAD állapotú a survivor
        }


        window.clear(sf::Color::Black);

        // Pálya megjelenítése
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);

                if (showFullMap) {
                    if (maze[i][j] == 1) {
                        cell.setFillColor(sf::Color(128, 128, 128)); // Fal
                    }
                    else {
                        cell.setFillColor(sf::Color::Black); // Üres hely
                    }
                }
                else {
                    // Fog of war alkalmazása
                    if (isCellVisible(survivor.position, j, i, SURVIVOR_VIEW_RADIUS, maze)) {
                        if (maze[i][j] == 1) {
                            cell.setFillColor(sf::Color(128, 128, 128)); // Fal
                        }
                        else {
                            cell.setFillColor(sf::Color::Black); // Üres hely
                        }
                    }
                    else if (isCellInKillerSight(killer, j, i, maze)) {
                        cell.setFillColor(sf::Color(50, 50, 50)); // Gyilkos látótávolságában
                    }
                    else {
                        cell.setFillColor(sf::Color(20, 20, 20)); // Köd
                    }
                }

                window.draw(cell);
            }
        }

        // Taskok megjelenítése (csak akkor, ha látja a survivor vagy a teljes pálya látható)
        for (Task& task : tasks) {
            bool isVisible = isTaskVisibleToSurvivor(task, survivor, maze);
            task.render(window, isVisible, showFullMap);
        }

        // Gyilkos megjelenítése
        sf::RectangleShape killerRect(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        killerRect.setPosition(killer.position.x, killer.position.y);
        killerRect.setFillColor(sf::Color::Red);
        window.draw(killerRect);

        // Túlélő megjelenítése az egészségügyi állapot alapján
        survivor.render(window);

        window.display();
    }

    return 0;
}
