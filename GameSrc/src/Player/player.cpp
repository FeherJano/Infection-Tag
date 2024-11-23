#include "player.hpp"
#include "../map.hpp"
#include "../game_constants.hpp"

namespace {
    enum MovementKeyIndex {
        UP_INDEX,
        DOWN_INDEX,
        LEFT_INDEX,
        RIGHT_INDEX
    };
}

Player::Player(float startX,
               float startY,
               float speed,
               std::array<sf::Keyboard::Key, 4> movementKeys) :
    position(startX, startY),
    lastDirection(1.0f, 0.0f),
    moveSpeed(speed),
    movementKeys(movementKeys)
{}

void Player::move(float deltaTime, const std::vector<std::vector<int>>& maze) {
    float dirX = 0.0f;
    float dirY = 0.0f;

    if (sf::Keyboard::isKeyPressed(movementKeys[UP_INDEX]))
        dirY -= 1.0f;
    if (sf::Keyboard::isKeyPressed(movementKeys[DOWN_INDEX]))
        dirY += 1.0f;
    if (sf::Keyboard::isKeyPressed(movementKeys[LEFT_INDEX]))
        dirX -= 1.0f;
    if (sf::Keyboard::isKeyPressed(movementKeys[RIGHT_INDEX]))
        dirX += 1.0f;
    
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

void Player::update(float deltaTime) {
    if (hitCooldownTimer > 0) {
        hitCooldownTimer -= deltaTime;
    }
}


Survivor::Survivor(float startX,
                   float startY,
                   std::array<sf::Keyboard::Key, 4> movementKeys) :
    Player(startX, startY, SURVIVOR_MOVE_SPEED, movementKeys),
    healthState(HEALTHY)
{}


void Survivor::getHit() {
    if (healthState == HEALTHY) {
        healthState = INJURED;
        speedBoostTimer = 3.0f;  // Speed boost for 3 seconds
    }
    else if (healthState == INJURED) {
        healthState = DYING;
        dyingTimer = maxDyingTime;  // Set dying timer
    }
}

void Survivor::heal() {

}

void Survivor::update(float deltaTime)  {
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

void Survivor::render(sf::RenderWindow& window) {
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

Killer::Killer(float startX,
               float startY,
               std::array<sf::Keyboard::Key, 4> movementKeys) :
    Player(startX, startY, KILLER_MOVE_SPEED, movementKeys)
{}

bool Killer::canHit() {
    return hitCooldownTimer <= 0.0f;
}

void Killer::hit(Survivor& survivor) {
    if (canHit()) {
        // Check that survivor is not in DYING or DEAD state
        if (survivor.healthState != DYING && survivor.healthState != DEAD) {
            survivor.getHit(); // Only hit if the survivor is not DYING or DEAD
            hitCooldownTimer = KILLER_HIT_COOLDOWN; // Set 1-second cooldown
            moveSpeed = KILLER_MOVE_SPEED * 0.3f; // Reduce speed after hit
        }
    }
}

void Killer::update(float deltaTime) {
    Player::update(deltaTime);

    if (hitCooldownTimer <= 0) {
        moveSpeed = KILLER_MOVE_SPEED; // Reset speed after cooldown
    }
}

void Killer::render(sf::RenderWindow& window) {
    sf::RectangleShape killerShape(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    killerShape.setPosition(position);
    killerShape.setFillColor(sf::Color::Red);
    window.draw(killerShape);
}

bool checkCollisionForKiller(Killer& killer, Survivor& survivor) {
    if (survivor.healthState == DYING || survivor.healthState == DEAD) {
        return false; // No collision for DYING or DEAD survivors
    }

    float distance = sqrt(pow(killer.position.x - survivor.position.x, 2) + pow(killer.position.y - survivor.position.y, 2));
    return distance < CELL_SIZE; // Killer is close enough to the survivor
}

void checkCollisionBetweenSurvivors(std::vector<Survivor>& survivors) {
    for (size_t i = 0; i < survivors.size(); ++i) {
        auto& survivor = survivors[i];
        if (survivor.healthState == DYING || survivor.healthState == INJURED) {
            for (size_t j = 0; j < survivors.size(); ++j) {
                if (i == j) {
                    continue;
                }
                auto& survivor2 = survivors[j];
                if (survivor2.healthState != DEAD) {
                    float distance = sqrt(pow(survivor2.position.x - survivor.position.x, 2) + pow(survivor2.position.y - survivor.position.y, 2));
                    if (distance < CELL_SIZE) {
                        survivor.heal();
                    }
                }
            }
        }
    }
}

