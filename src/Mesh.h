#pragma once

#include <cstddef>
#include <vector>

#include <glad/glad.h>

struct VertexAttribute {
    GLenum type;
    size_t numElements;
    size_t sizeOfType;
};

using VertexAttribSet = std::vector<VertexAttribute>;

class Mesh {
private:
    unsigned int vao;
    unsigned int numVertices;
public:
    Mesh(float* data, int numVertices, const VertexAttribSet& attribSet);

    void render() const;
};