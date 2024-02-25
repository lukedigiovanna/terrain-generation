#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <memory>

#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

struct Part {
    Mesh& mesh;
    Texture& texture;
    glm::vec3 offsetPosition;
    glm::vec3 scale;
};

using Model = std::vector<std::unique_ptr<Part>>;

struct WorldObject {
    Model& model;

    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 velocity;
    float angle;

    void render(Shader& objectShader) const;
};

namespace models {
    extern Model TREE;

    void initialize();
};