#include <SFML/Graphics.hpp>
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
    int numSurvivors = 2; // Example with 1 survivor
    placeTasks(tasks, maze, numSurvivors);

    // Add players
    Killer killer(1.0f * CELL_SIZE, 1.0f * CELL_SIZE,
        { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D });
    std::vector<Survivor> survivors;
    survivors.emplace_back(Survivor((WIDTH - 2.0f) * CELL_SIZE, (HEIGHT - 2.0f) * CELL_SIZE,
        { sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right }));
    survivors.emplace_back(Survivor((WIDTH - 2.0f) * CELL_SIZE, (HEIGHT - 2.0f) * CELL_SIZE,
        { sf::Keyboard::I, sf::Keyboard::K, sf::Keyboard::J, sf::Keyboard::L }));

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
                for (auto& survivor : survivors) {
                    survivor.healthState = HEALTHY;
                }
            }

            // Handle "Q" key press
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q) {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        killer.move(deltaTime, maze);

        for (auto& survivor : survivors) {
            survivor.move(deltaTime, maze);
        }

        // Update tasks and progress
        for (Task& task : tasks) {
            int numSurvivorsOnTask = 0;
            for (auto& survivor : survivors) {
                numSurvivorsOnTask += isSurvivorOnTask(survivor, task) ? 1 : 0;
            }
            task.update(deltaTime, numSurvivorsOnTask);
        }

        // Update players
        killer.update(deltaTime);
        for (auto& survivor : survivors) {
            survivor.update(deltaTime);
        }

        // Check collision between killer and survivor
        for (auto& survivor : survivors) {
            if (checkCollisionForKiller(killer, survivor) && killer.canHit()) {
                killer.hit(survivor); // Hit survivor if not in DYING or DEAD state
            }
        }
        checkCollisionBetweenSurvivors(survivors);

        window.clear(sf::Color::Black);
        renderMap(window, maze, killer, survivors, showFullMap);

        
        // Render tasks (only if visible to survivor or full map view)
        for (Task& task : tasks) {
            bool isVisible = isTaskVisibleToAnySurvivor(task, survivors, maze);
            task.render(window, isVisible, showFullMap);
        }

        for (auto& survivor : survivors) {
            survivor.render(window);
        }
        killer.render(window);

        window.display();
    }

    return 0;
}
