#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

int main() {
    sf::ContextSettings settings;
    settings.depthBits = 24;     
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 4; 
    settings.minorVersion = 1;

    sf::RenderWindow window(
        sf::VideoMode(1024, 768),
        "Maze Runner 3D - Tappa 01",
        sf::Style::Default,
        settings
    );

    window.setFramerateLimit(60);

    std::cout << "Welcome to MazeRunner" << std::endl;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        window.clear(sf::Color(30, 30, 35));
        window.display();
    }

    return 0;
}