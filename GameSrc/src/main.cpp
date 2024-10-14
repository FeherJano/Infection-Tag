/*
#include "SFML/Graphics.hpp"
#include "player.hpp"
#include "map.hpp"
#include "task.hpp"
*/
#include "Server/server.hpp"
const unsigned short myPort = 8085;

int main() {
    
    CatGameServer * server = new CatGameServer(8085);
    server->ServerFunction();


    return 0;
}
