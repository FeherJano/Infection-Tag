#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "map.hpp"
#include "game_constants.hpp"

using namespace std;


class Player {
public:
    sf::Vector2f position;
    sf::Vector2f lastDirection;
    float moveSpeed;
    float hitCooldownTimer = 0.0f;

    Player(float startX, float startY, float speed);
    void move(float dirX, float dirY, float deltaTime, const std::vector<std::vector<int>>& maze);
    virtual void update(float deltaTime);
    virtual void render(sf::RenderWindow& window) = 0;
};

class Survivor : public Player {
public:
    HealthState healthState;
    float speedBoostTimer = 0.0f;
    float dyingTimer = 0.0f;  // Timer for the DYING state
    const float maxDyingTime = 30.0f;  // Max time for the DYING state

    Survivor(float startX, float startY);
    void getHit();
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

class Killer : public Player {
public:
    Killer(float startX, float startY);
    bool canHit();
    void hit(Survivor& survivor);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

bool checkCollisionBetween(Killer& killer, Survivor& survivor);


#endif
