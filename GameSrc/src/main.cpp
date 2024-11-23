#include "Player/player.hpp"
#include "map.hpp"
#include "task.hpp"
  
#include "WindowApp/WindowApp.hpp"

const unsigned width = 800, height = 800;
int main() {
    asio::io_context ioContext;
    WindowApp game(ioContext, width, height);
    game.main();

    return 0;
}
