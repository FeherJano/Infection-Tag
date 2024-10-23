#pragma once
#include "uiElement.hpp"


class Button : public uiElement {
private:
	sf::RectangleShape body;
	sf::Text label;
	std::unique_ptr< sf::Font> font;
	
public:
	Button(const sf::RenderWindow &renderWindow, sf::Vector2f pos, sf::Vector2f size, std::string text, uint16_t id);
	~Button();

	int elementFunction(const sf::Event &e)override;
	void renderElement(sf::RenderWindow* window)const override;

	static const float defaultButtonWidth;
	static const sf::Color baseColor;
	static const sf::Color textColor;
	static const sf::Color activatedColor1;
	static const sf::Color activatedColor2;
};