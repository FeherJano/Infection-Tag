#pragma once
#include <SFML/Graphics.hpp>

class Player {
public:
    sf::Vector2f position;

    Player(float startX, float startY) {
        position = sf::Vector2f(startX, startY);
    }

    void move(float dirX, float dirY, float deltaTime) {
        position.x += dirX * deltaTime;
        position.y += dirY * deltaTime;
    }
};
