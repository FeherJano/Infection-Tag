#include "Button.hpp"
#include "../../Utility/logging.hpp"

//STATIC CLASS VARIABLES DEFINED HERE

const float Button::defaultButtonWidth = 125;
const sf::Color Button::baseColor = sf::Color::Magenta;
const sf::Color Button::textColor = sf::Color::Blue;
const sf::Color Button::activatedColor1 = sf::Color::White; //mouse on the button
const sf::Color Button::activatedColor2 = sf::Color::Yellow; //user clicked on the button

//Button class constructors and destructors


Button::Button(const sf::RenderWindow& renderWindow, sf::Vector2f pos, sf::Vector2f size, std::string text, uint16_t id) :
	uiElement(&renderWindow, pos, size, id), body(sf::RectangleShape(size)), label(sf::Text()), font(new sf::Font())
{
	if (!font->loadFromFile("./src/Utility/Fonts/Raleway-Regular.ttf")) { // Use a valid path to a .ttf file
		logErr(noSuitableFontException().what())
		exit(err_Fatal);
	}
	try {
		label.setFont(*font);
		label.setString(text);
	}
	catch (std::exception e) {
		logErr(e.what());
		exit(err_Fatal);
	}
	
	body.setPosition(pos);
	sf::Vector2f labelPos = pos; //the text starts at the upper left side of the box
	//centering the text
	labelPos.x += label.findCharacterPos(text.size() ).x / 2; //the text is moved to the right with the half of the last characters x position
	labelPos.y += label.getCharacterSize() / 4; //the text is moved to the bottom with one quarter of the current char size
	label.setPosition(labelPos);
	body.setFillColor(Button::baseColor);
	label.setFillColor(Button::textColor);
}

Button::~Button() {
}


//Button class function implementations


int Button::elementFunction(const sf::Event& e) {
	sf::Vector2i mousePos = sf::Mouse::getPosition(*mainWindow);

	if (e.mouseButton.button == sf::Mouse::Left &&
		isMouseThere(mousePos,getPosition(),getPosition()+getSize())
		){
		body.setFillColor(Button::activatedColor2);
		return 1;
	}
	if (isMouseThere(mousePos, getPosition(), getPosition() + getSize())) {
		body.setFillColor(Button::activatedColor1);
		return 0;
	}
	body.setFillColor(Button::baseColor);
	return 0;
}

void Button::renderElement(sf::RenderWindow* window)const {
	window->draw(body);
	window->draw(label);
}