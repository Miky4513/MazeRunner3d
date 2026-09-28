cmake_minimum_required(VERSION 3.16)
project(MazeRunner3D VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trova la libreria SFML e OpenGL installate nel sistema
find_package(SFML COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

# Include tutti i sorgenti in src/
file(GLOB SOURCES "src/*.cpp")

# Definisce l'eseguibile
add_executable(MazeRunner3D ${SOURCES})

# Configura i collegamenti alle librerie
target_link_libraries(MazeRunner3D PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    OpenGL::GL
)
