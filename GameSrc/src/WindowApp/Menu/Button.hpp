#pragma once
#include "uiElement.hpp"


class Button : public uiElement {
private:
	sf::RectangleShape body;
	sf::Text label;
	sf::Font *font;
	
public:
	Button(sf::Vector2f pos, sf::Vector2f size, std::string text, uint16_t id);
	~Button();

	int elementFunction(const sf::Event& e)override;
	void renderElement(sf::RenderWindow* window)const override;
};
