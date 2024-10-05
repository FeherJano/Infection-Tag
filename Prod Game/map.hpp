#ifndef MAP_HPP
#define MAP_HPP

#include <vector>
#include <SFML/Graphics.hpp>
#include "structures.hpp"
#include "player.hpp"
#include "game_constants.hpp"

using namespace std;

class Player;
class Killer;
class Survivor;

bool isBlocked(int gridX, int gridY, const std::vector<std::vector<int>>& maze);
void placeObjects(std::vector<std::vector<int>>& maze);
void renderMap(sf::RenderWindow& window, const std::vector<std::vector<int>>& maze, const Killer& killer, const std::vector<Survivor>& survivors, bool showFullMap);
bool checkCollision(sf::Vector2f position, float playerSize, const std::vector<std::vector<int>>& maze);
bool canPlaceObject(const std::vector<vector<int>>& maze, int x, int y, int width, int height);
bool isCellVisible(sf::Vector2f playerPos, int gridX, int gridY, float viewRadius, const vector<vector<int>>& maze);
bool isCellInKillerSight(const Player& killer, int gridX, int gridY, const std::vector<std::vector<int>>& maze);


#endif
