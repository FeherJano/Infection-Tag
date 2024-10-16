#include "WindowApplication.hpp"



void WindowApplication::processInput() {
	sf::Event ev;

	while (mainWindow->pollEvent(ev)) {
		if (ev.type == sf::Event::Closed) {
			this->mainWindow->close();
		}

	}

	
}



int WindowApplication::main() {

	while (mainWindow->isOpen()) {
	
		this->processInput();
		mainWindow->clear(sf::Color::Black);
		mainWindow->display();

	}
	return 0;
}


