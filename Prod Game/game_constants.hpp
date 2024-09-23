#ifndef GAME_CONSTANTS_HPP
#define GAME_CONSTANTS_HPP

// Map dimensions
const int WIDTH = 90;  // Map width
const int HEIGHT = 90; // Map height
const int CELL_SIZE = 10; // Cell size in the display
const int GRID_ROWS = 3; // Number of blocks in rows
const int GRID_COLS = 3; // Number of blocks in columns

// Gameplay constants
const float SURVIVOR_VIEW_RADIUS = 20.0f; // Survivor vision radius (in cells)
const float KILLER_VIEW_DISTANCE = 25.0f; // Killer's sight distance (in cells)
const float SURVIVOR_MOVE_SPEED = 100.0f; // Survivor speed
const float KILLER_MOVE_SPEED = 120.0f; // Killer speed
const float KILLER_HIT_COOLDOWN = 1.0f; // Killer attack cooldown
const float SURVIVOR_HIT_SPEED_BOOST = 100.0f; // Speed boost for survivors when hit
const int HEALTH_STATE_COUNT = 3; // Number of health states

// Health states for survivors
enum HealthState { HEALTHY, INJURED, DYING, DEAD };

#endif
