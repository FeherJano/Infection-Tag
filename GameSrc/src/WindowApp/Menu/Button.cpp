#include "Button.hpp"


Button::Button(sf::Vector2f pos, sf::Vector2f size, std::string text, uint16_t id) : uiElement(pos, size, id), body(sf::RectangleShape(size)), label(sf::Text()), font(new sf::Font())
{
	if (!font->loadFromFile("./src/Utility/Fonts/Gerhaus-PK69E.ttf")) { // Use a valid path to a .ttf file
		throw new std::exception("No loadable font found. Terminating...");
	}
	label.setFont(*font);
	label.setString(text);
	body.setPosition(pos);
	sf::Vector2f labelPos = pos;
	//centering the text
	labelPos.x += size.x / 2 - (float) ((text.length()/2 ) * label.getCharacterSize());
	labelPos.y += size.y / 2 - (float) (label.getCharacterSize() / 2);
	label.setPosition(labelPos);
	body.setFillColor(sf::Color::Magenta);
	label.setFillColor(sf::Color::Blue);
}

Button::~Button() {
	delete font;
}


int Button::elementFunction(const sf::Event &e) {
	if (e.mouseButton.button == sf::Mouse::Left &&
		isMouseThere(e.mouseButton,getPosition(),getPosition()+getSize())
		){
		body.setFillColor(sf::Color::White);
		return 1;
	}
	return 0;
}

void Button::renderElement(sf::RenderWindow* window)const {
	window->draw(body);
	window->draw(label);
}