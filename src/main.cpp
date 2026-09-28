#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

int main() {
    // Impostazioni del contesto OpenGL (Slide 01/02)
    sf::ContextSettings settings;
    settings.depthBits = 24;      // Risoluzione Z-Buffer per il 3D
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 4;    // OpenGL 4.1
    settings.minorVersion = 1;

    // Creazione finestra (VideoMode, Titolo, Stile, Settings) - Slide 02
    sf::RenderWindow window(
        sf::VideoMode(1024, 768),
        "Maze Runner 3D - Tappa 01",
        sf::Style::Default,
        settings
    );

    window.setFramerateLimit(60);

    std::cout << "Maze Runner 3D: Inizializzazione completata!" << std::endl;

    // Event Loop classico (Schema EDP - Slide 01 & 02)
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Gestione chiusura finestra (Slide 01 e 02)
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // Gestione pressione tasto ESC (Slide 02)
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // Pulizia dello schermo (Clear) - Slide 02
        window.clear(sf::Color(30, 30, 35));

        // Presentazione a schermo (Double Buffering / Display) - Slide 02
        window.display();
    }

    return 0;
}