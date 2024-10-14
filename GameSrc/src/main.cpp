#include "SFML/Graphics.hpp"
#include "player.hpp"
#include "map.hpp"
#include "task.hpp"

using namespace std;

int main() {
    srand(time(0));

    // 2D map, initially empty (0 = empty space, 1 = wall)
    std::vector<std::vector<int>> maze(HEIGHT, std::vector<int>(WIDTH, 0));

    // Generate map with objects and buildings
    placeObjects(maze);

    // Add tasks
    std::vector<Task> tasks;
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
        renderMap(window, maze, killer, survivor, showFullMap);

        
        // Render tasks (only if visible to survivor or full map view)
        for (Task& task : tasks) {
            bool isVisible = isTaskVisibleToSurvivor(task, survivor, maze);
            task.render(window, isVisible, showFullMap);
        }

        survivor.render(window);
        killer.render(window);

        window.display();
    }

    return 0;
}
