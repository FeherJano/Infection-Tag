#pragma once
#include "SFML/Graphics.hpp"


class uiElement {
protected:
    const sf::RenderWindow* mainWindow;
private:
    sf::Vector2f position;
    sf::Vector2f size;
    uint16_t id;
   
public:

    uiElement(const sf::RenderWindow * renderWindow,sf::Vector2f pos, sf::Vector2f sz, uint16_t id) : mainWindow(renderWindow), position(pos), size(sz), id(id) {}

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


    static bool isMouseThere(const sf::Vector2i& mousePos, const sf::Vector2f& topLeft, const sf::Vector2f& bottomRight) {
        return mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
            mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y;
    }

};