#pragma once
#include "uiElement.hpp"


class TextBox : public uiElement {
private:
	sf::RectangleShape body;
	sf::Text label;
	std::unique_ptr< sf::Font> font;
	std::string inputText;
	bool isActive;

public:
	TextBox(const sf::RenderWindow& renderWindow, sf::Vector2f pos, sf::Vector2f size, std::string text, uint16_t id);
	~TextBox();

	int elementFunction(const sf::Event& e)override;
	void renderElement(sf::RenderWindow* window)const override;

	void processTyping(const sf::Event& e);
	std::string getText() const;

	static const float defaultBoxWidth;
	static const sf::Color baseColor;
	static const sf::Color textColor;
	static const sf::Color activatedColor1;
	static const sf::Color activatedColor2;
};
#pragma once
