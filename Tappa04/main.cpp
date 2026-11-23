#include "glad/glad.h"
#include <SFML/Window.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <random>

#include "hotshaders.hh"
#include "matrices.hh"
#include "rawmouse.hh"


const int MAZE_W = 21;
const int MAZE_H = 21;
const float WALL_HEIGHT = 2.0f;
const float PLAYER_RADIUS = 0.2f;


class Setup
{
public:
    static const int W = 800;
    static const int H = 600;
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
        window = new sf::Window(sf::VideoMode({W, H}),
            "MazeRunner3D - Tappa 04",
            sf::Style::Default, sf::State::Windowed, settings);
        window->setVerticalSyncEnabled(true);
        if (!window->setActive(true)) { std::cerr << "SFML GL activation failed" << std::endl; exit(1); }
        if (!gladLoadGLLoader((GLADloadproc)sf::Context::getFunction)) { std::cerr << "glad failed" << std::endl; exit(1); }
    }
    ~Setup () { delete window; }
};


class Camera
{
public:
    glm::vec3 position = {1.5f, 1.0f, 1.5f};
    float phi_deg = 0.0f;
    float theta_deg = 0.0f;

    glm::vec3 forward () const
    {
        float p = glm::radians(phi_deg), t = glm::radians(theta_deg);
        return glm::vec3(sin(p)*cos(t), sin(t), -cos(p)*cos(t));
    }
    glm::vec3 flat_forward () const
    { return glm::normalize(glm::vec3(forward().x, 0.0f, forward().z)); }
    glm::vec3 right () const
    {
        float p = glm::radians(phi_deg);
        return glm::vec3(cos(p), 0.0f, sin(p));
    }
    void rotate (sf::Vector2f d, float s = 0.1f)
    {
        phi_deg   += d.x * s;
        theta_deg -= d.y * s;
        theta_deg = std::clamp(theta_deg, -89.0f, 89.0f);
    }
    glm::mat4 view () const
    { return glm::lookAt(position, position + forward(), glm::vec3(0,1,0)); }
};


class Maze
{
public:
    std::vector<std::vector<int>> cells;

    Maze () : cells(MAZE_H, std::vector<int>(MAZE_W, 1)) { generate(); }

    void generate ()
    {
        for (auto& row : cells) std::fill(row.begin(), row.end(), 1);

        std::mt19937 rng(std::random_device{}());
        std::vector<std::pair<int,int>> dirs = {{0,-2},{0,2},{-2,0},{2,0}};
        std::vector<std::pair<int,int>> stack;
        stack.push_back({1,1});
        cells[1][1] = 0;

        while (!stack.empty()) {
            auto [x,y] = stack.back();
            std::vector<std::pair<int,int>> nb;
            for (auto [dx,dy] : dirs) {
                int nx = x+dx, ny = y+dy;
                if (nx>0 && nx<MAZE_W-1 && ny>0 && ny<MAZE_H-1 && cells[ny][nx]==1)
                    nb.push_back({nx,ny});
            }
            if (!nb.empty()) {
                std::uniform_int_distribution<int> dist(0, nb.size()-1);
                auto [nx,ny] = nb[dist(rng)];
                cells[(y+ny)/2][(x+nx)/2] = 0;
                cells[ny][nx] = 0;
                stack.push_back({nx,ny});
            } else {
                stack.pop_back();
            }
        }
    }
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
        glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(float), points.data(), GL_STATIC_DRAW);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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

    void draw_cube (glm::mat4 model, glm::vec3 color)
    {
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(color_loc, 1, glm::value_ptr(color));
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }

    void draw (const Camera& cam, const Maze& maze, float ar)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = cam.view();
        glm::mat4 proj = glm::perspective(glm::radians(70.0f), ar, 0.1f, 100.0f);
        glm::mat4 vp = proj * view;
        glUniformMatrix4fv(vp_loc, 1, GL_FALSE, glm::value_ptr(vp));

        glBindVertexArray(vao);

        for (int y = 0; y < MAZE_H; ++y) {
            for (int x = 0; x < MAZE_W; ++x) {
                if (maze.cells[y][x] == 1) {
                    glm::mat4 m = fcg::translation(x + 0.5f, WALL_HEIGHT*0.5f, y + 0.5f)
                                * fcg::scaling(1.0f, WALL_HEIGHT, 1.0f);
                    draw_cube(m, {0.55f, 0.55f, 0.55f});
                } else {
                    glm::mat4 m = fcg::translation(x + 0.5f, -0.05f, y + 0.5f)
                                * fcg::scaling(1.0f, 0.1f, 1.0f);
                    draw_cube(m, {0.35f, 0.22f, 0.12f});
                }
            }
        }
    }
};


int main ()
{
    Setup setup;
    sf::Window& window = *setup.window;
    window.setMouseCursorGrabbed(true);
    window.setMouseCursorVisible(false);

    fcg::Shaders shaders("resources/shaders/simple_model_vp.vert",
                         "resources/shaders/simple_color.frag");
    shaders.use();

    Scene scene(shaders);
    Camera camera;
    fcg::RawMouse raw_mouse;
    Maze maze;

    glEnable(GL_DEPTH_TEST);

    sf::Clock delta_clock;
    bool running = true;
    while (running) {
        float dt = std::min(delta_clock.restart().asSeconds(), 0.1f);

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) running = false;
            else if (const auto* r = event->getIf<sf::Event::Resized>())
                glViewport(0, 0, r->size.x, r->size.y);
            else if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {
                if (k->scancode == sf::Keyboard::Scancode::Escape) running = false;
                else if (k->scancode == sf::Keyboard::Scancode::R) {
                    maze.generate();
                    camera.position = {1.5f, 1.0f, 1.5f};
                }
            }
            else if (const auto* raw = event->getIf<sf::Event::MouseMovedRaw>())
                raw_mouse.event(*raw);
        }

        camera.rotate(raw_mouse.delta());

        glm::vec3 move(0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) move += camera.flat_forward();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) move -= camera.flat_forward();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) move += camera.right();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) move -= camera.right();
        if (glm::length(move) > 0.0f)
            camera.position += glm::normalize(move) * 3.0f * dt;

        sf::Vector2u s = window.getSize();
        scene.draw(camera, maze, (float)s.x / (float)s.y);
        window.display();
    }
    return 0;
}
