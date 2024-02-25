#include "WorldObject.h"

#include "Mesh.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>

void WorldObject::render(Shader& objectShader) const {
    objectShader.use();
    for (const auto & part : model) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos + part->offsetPosition);
        model = glm::scale(model, scale + part->scale);
        objectShader.setMatrix4("model", model);
        part->texture.bind();
        part->mesh.render();
    };
}

Model models::TREE;

void models::initialize() {
    TREE.push_back(
        std::make_unique<Part>(Part{*meshes::CUBE, *textures::WOOD, glm::vec3(0, 2.5, 0), glm::vec3(0.4f, 5, 0.4f)})
    );
    TREE.push_back(
        std::make_unique<Part>(Part{*meshes::CUBE, *textures::GRASS, glm::vec3(0, 6.5, 0), glm::vec3(3, 3, 3)})
    );
}