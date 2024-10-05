#ifndef TASK_HPP
#define TASK_HPP

#include <SFML/Graphics.hpp>

using namespace std;

class Survivor;

class Task {
public:
    sf::Vector2f position;
    float progress = 0.0f;
    const float maxProgress = 30.0f;

    Task(float startX, float startY);
    void update(float deltaTime, int numSurvivorsOnTask);
    void render(sf::RenderWindow& window, bool isVisible, bool showFullMap);
};

void placeTasks(std::vector<Task>& tasks, std::vector<std::vector<int>>& maze, int numSurvivors);
bool isSurvivorOnTask(const Survivor& survivor, const Task& task);
bool isTaskVisibleToAnySurvivor(const Task& task, const std::vector<Survivor>& survivors, const std::vector<std::vector<int>>& maze);

#endif
