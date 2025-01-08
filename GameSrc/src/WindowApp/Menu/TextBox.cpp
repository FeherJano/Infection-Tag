#include "TextBox.hpp"
#include "../../Utility/logging.hpp"

//STATIC CLASS VARIABLES DEFINED HERE

const float TextBox::defaultBoxWidth = 125;
const sf::Color TextBox::baseColor = sf::Color::Magenta;
const sf::Color TextBox::textColor = sf::Color::Blue;
const sf::Color TextBox::activatedColor1 = sf::Color::White; //mouse on the button
const sf::Color TextBox::activatedColor2 = sf::Color::Yellow; //user clicked on the button

//Button class constructors and destructors


TextBox::TextBox(const sf::RenderWindow& renderWindow, sf::Vector2f pos, sf::Vector2f size, std::string text, uint16_t id) :
	uiElement(&renderWindow, pos, size, id), body(sf::RectangleShape(size)), label(sf::Text()), font(new sf::Font()), inputText(), isActive(false)
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
	labelPos.y += label.getCharacterSize() / 4; //the text is moved to the bottom with one quarter of the current char size
	label.setPosition(labelPos);
	body.setFillColor(TextBox::baseColor);
	label.setFillColor(TextBox::textColor);
}

TextBox::~TextBox() {
}


//Handle text input

void TextBox::processTyping(const sf::Event& e) {
	if (e.text.unicode < 128) {			// Check for ASCII range
		if (e.text.unicode == '\b') {		// Handle backspace
			if (!inputText.empty()) {
				inputText.pop_back();
			}
		}
		else {
			inputText += static_cast<char>(e.text.unicode);
		}
		label.setString(inputText);		// Update the text displayed
	}
}

std::string TextBox::getText() const{
	return inputText;
}


//Button class function implementations with Textbox features


int TextBox::elementFunction(const sf::Event& e) {
	sf::Vector2i mousePos = sf::Mouse::getPosition(*mainWindow);

	if (e.mouseButton.button == sf::Mouse::Left &&		// Activated by a left click in the area
		isMouseThere(mousePos, getPosition(), getPosition() + getSize()) &&
		sf::Mouse::isButtonPressed(e.mouseButton.button)
		) {
		body.setFillColor(TextBox::activatedColor2);
		isActive = true;
		return 1;
	}
	if (isActive && e.type == sf::Event::TextEntered) {		// If active you can write
		processTyping(e);
		return 1;
	}
	if (e.mouseButton.button == sf::Mouse::Left &&		// Deactivate by a left click anywere but in the area
		!isMouseThere(mousePos, getPosition(), getPosition() + getSize()) &&
		sf::Mouse::isButtonPressed(e.mouseButton.button)
		) {
		body.setFillColor(TextBox::baseColor);
		isActive = false;
		return 0;
	}
	if (isMouseThere(mousePos, getPosition(), getPosition() + getSize())) {		// Other color if cursor in the area
		body.setFillColor(TextBox::activatedColor1);
		return 0;
	}
	return 0;
}


void TextBox::renderElement(sf::RenderWindow* window)const {
	window->draw(body);
	window->draw(label);
}