#include "WorldObject.h"

#include "Mesh.h"
#include "Texture.h"

Model models::TREE;

void models::initialize() {
    TREE.push_back(
        std::make_unique<Part>(Part{*meshes::CUBE, *textures::WOOD, glm::vec3(0, 2.5, 0), glm::vec3(0.4f, 5, 0.4f)})
    );
    TREE.push_back(
        std::make_unique<Part>(Part{*meshes::CUBE, *textures::GRASS, glm::vec3(0, 6.5, 0), glm::vec3(3, 3, 3)})
    );
}