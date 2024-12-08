#ifndef TASK_HPP
#define TASK_HPP

#include <SFML/Graphics.hpp>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class Survivor;

class Task {
public:
    sf::Vector2f position;
    float progress = 0.0f;
    float maxProgress = 30.0f;

    Task(float startX, float startY);
    void update(float deltaTime, int numSurvivorsOnTask);
    void render(sf::RenderWindow& window, bool isVisible, bool showFullMap) const;

    // Konverzió JSON-re
    json to_json() const {
        return {
            {"position", {position.x, position.y}},
            {"progress", progress},
            {"maxProgress", maxProgress}
        };
    }

    // Konverzió JSON-ből
    void from_json(const json& j) {
        position.x = j.at("position").at(0);
        position.y = j.at("position").at(1);
        progress = j.at("progress");
        maxProgress = j.at("maxProgress");
    }


};

void placeTasks(std::vector<Task>& tasks, std::vector<std::vector<int>>& maze, int numSurvivors);
bool isSurvivorOnTask(const Survivor& survivor, const Task& task);
bool isTaskVisibleToAnySurvivor(const Task& task, const std::vector<Survivor>& survivors, const std::vector<std::vector<int>>& maze);

#endif
