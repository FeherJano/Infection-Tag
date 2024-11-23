#include "task.hpp"
#include "Player/player.hpp"
#include "map.hpp"
#include <random>

Task::Task(float startX, float startY) : position(startX, startY) {}

void Task::update(float deltaTime, int numSurvivorsOnTask) {
    if (numSurvivorsOnTask > 0) {
        progress += deltaTime * numSurvivorsOnTask; // More survivors speed up the task
        if (progress > maxProgress) {
            progress = maxProgress;
        }
    }
}

void Task::render(sf::RenderWindow& window, bool isVisible, bool showFullMap) {
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

void placeTasks(std::vector<Task>& tasks, std::vector<std::vector<int>>& maze, int numSurvivors) {
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

bool isSurvivorOnTask(const Survivor& survivor, const Task& task) {
    return (survivor.healthState == HEALTHY || survivor.healthState == INJURED) &&
        (survivor.position.x >= task.position.x && survivor.position.x <= task.position.x + CELL_SIZE * 3 &&
            survivor.position.y >= task.position.y && survivor.position.y <= task.position.y + CELL_SIZE * 3);
}

bool isTaskVisibleToAnySurvivor(const Task& task, const std::vector<Survivor>& survivors, const std::vector<std::vector<int>>& maze) {
    bool visible = false;
    for (auto& survivor : survivors) {
        if (isCellVisible(survivor.position, task.position.x / CELL_SIZE, task.position.y / CELL_SIZE, SURVIVOR_VIEW_RADIUS, maze)) {
            visible = true;
            break;
        }
    }
    return visible;
}
