#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include "../map.hpp"
#include "../game_constants.hpp"
#include "../Networking/Client/Client.hpp"


class Player {

    float dirX = 0.0f;
    float dirY = 0.0f;
    std::array<sf::Keyboard::Key, 4> movementKeys;

public:
    sf::Vector2f position;
    sf::Vector2f lastDirection;
    float moveSpeed;
    float hitCooldownTimer = 0.0f;

    Player(float startX, float startY, float speed, std::array<sf::Keyboard::Key, 4>);
    void move(float deltaTime, const std::vector<std::vector<int>>& maze);
    virtual void update(float deltaTime);
    virtual void render(sf::RenderWindow& window) = 0;
};

class Survivor : public Player {
public:
    HealthState healthState;
    float speedBoostTimer = 0.0f;
    float dyingTimer = 0.0f;  // Timer for the DYING state
    const float maxDyingTime = 30.0f;  // Max time for the DYING state

    Survivor(float startX, float startY, std::array<sf::Keyboard::Key, 4>);
    void getHit();
    void heal();
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

class Killer : public Player {
public:
    Killer(float startX, float startY, std::array<sf::Keyboard::Key, 4>);
    bool canHit();
    void hit(Survivor& survivor);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

bool checkCollisionForKiller(Killer& killer, Survivor& survivor);
void checkCollisionBetweenSurvivors(std::vector<Survivor>& survivors);

#endif
