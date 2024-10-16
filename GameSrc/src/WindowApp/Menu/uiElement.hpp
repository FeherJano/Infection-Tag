#pragma once
#include "SFML/Graphics.hpp"


class uiElement {
private:
    sf::Vector2f position;
    sf::Vector2f size;
    uint16_t id;

public:

    uiElement(sf::Vector2f pos, sf::Vector2f sz, uint16_t id) : position(pos), size(sz), id(id) {}

    sf::Vector2f getPosition() const {
        return position;
    }

    sf::Vector2f getSize() const {
        return size;
    }

    uint16_t getId() const {
        return id;
    }

    void setPosition(const sf::Vector2f& pos) {
        position = pos;
    }
    void setSize(const sf::Vector2f& sz) {
        size = sz;
    }

    virtual int elementFunction(const sf::Event& e) {
        return 0;
    }

    virtual void renderElement(sf::RenderWindow* window) const {

    }

    static bool isMouseThere(const sf::Event::MouseButtonEvent& mousePos, const sf::Vector2f& p1, const sf::Vector2f& p2) {
        return mousePos.x > p1.x && mousePos.y > p1.y &&
            mousePos.x < p2.x && mousePos.y < p2.y;
    }
};