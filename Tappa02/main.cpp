#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "hotshaders.hh"
#include "matrices.hh"


class Setup
{
public:
    sf::Window* window;
    Setup ()
    {
        sf::ContextSettings settings;
        settings.depthBits = 32;
        settings.stencilBits = 8;
        settings.antiAliasingLevel = 4;
        settings.attributeFlags = sf::ContextSettings::Attribute::Core;
        settings.majorVersion = 4;
        settings.minorVersion = 1;
        window = new sf::Window(sf::VideoMode({800, 600}),
            "MazeRunner3D - Tappa 02",
            sf::Style::Default, sf::State::Windowed, settings);
        window->setVerticalSyncEnabled(true);
        if (!window->setActive(true)) { std::cerr << "SFML GL activation failed" << std::endl; exit(1); }
        if (!gladLoadGLLoader((GLADloadproc)sf::Context::getFunction)) { std::cerr << "glad failed" << std::endl; exit(1); }
    }
    ~Setup () { delete window; }
};


class Scene
{
private:
    std::vector<float> points;
    std::vector<unsigned int> indices;
    GLuint vbo, ebo, vao;
    GLint model_loc, vp_loc, color_loc;

public:
    Scene (fcg::Shaders& shaders) { load(shaders); }
    ~Scene () { clean(); }

    void load (fcg::Shaders& shaders)
    {
        // 8 vertici del cubo
        points = {
            -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
            -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f
        };
        indices = {
            0,1,2, 2,3,0,  1,5,6, 6,2,1,
            5,4,7, 7,6,5,  4,0,3, 3,7,4,
            3,2,6, 6,7,3,  4,5,1, 1,0,4
        };

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float),
                     points.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     indices.data(), GL_STATIC_DRAW);

        model_loc = glGetUniformLocation(shaders.program, "model");
        vp_loc    = glGetUniformLocation(shaders.program, "vp");
        color_loc = glGetUniformLocation(shaders.program, "u_color");
    }

    void clean ()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    void draw (float time, float ar)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = fcg::rotation_y(time * 50.0f) * fcg::rotation_x(time * 30.0f);

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 1.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), ar, 0.1f, 100.0f);
        glm::mat4 vp = proj * view;

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(vp_loc,    1, GL_FALSE, glm::value_ptr(vp));
        glUniform3f(color_loc, 0.9f, 0.6f, 0.2f);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
};


int main ()
{
    Setup setup;
    sf::Window& window = *setup.window;

    fcg::Shaders shaders("resources/shaders/simple_model_vp.vert",
                         "resources/shaders/simple_color.frag");
    shaders.use();

    Scene scene(shaders);
    glEnable(GL_DEPTH_TEST);

    sf::Clock clock;
    bool running = true;
    while (running) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) running = false;
            else if (const auto* r = event->getIf<sf::Event::Resized>())
                glViewport(0, 0, r->size.x, r->size.y);
            else if (const auto* k = event->getIf<sf::Event::KeyPressed>())
                if (k->scancode == sf::Keyboard::Scancode::Escape) running = false;
        }
        sf::Vector2u s = window.getSize();
        float ar = (float)s.x / (float)s.y;
        scene.draw(clock.getElapsedTime().asSeconds(), ar);
        window.display();
    }
    return 0;
}
