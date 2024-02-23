#include "Mesh.h"

#include <glad/glad.h>

#include <algorithm>

Mesh::Mesh(float* data, int numVertices, const VertexAttribSet& attribSet) {
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);
    size_t numElements = 0;
    std::for_each(attribSet.begin(), attribSet.end(), [&numElements](const VertexAttribute& attrib) {
        numElements += attrib.numElements;
    });
    size_t byteOffset = 0;
    for (int i = 0; i < attribSet.size(); i++) {
        glVertexAttribPointer(
            i, 
            attribSet[i].numElements, 
            attribSet[i].type, 
            GL_FALSE, 
            numElements * attribSet[i].sizeOfType, 
            (void*) byteOffset
        );
        byteOffset += attribSet[i].numElements * attribSet[i].sizeOfType;
        glEnableVertexAttribArray(i);
    }
    glBufferData(GL_ARRAY_BUFFER, byteOffset * numVertices, data, GL_STATIC_DRAW);
    this->numVertices = numVertices;
}

void Mesh::render() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

float cubeVertices[288] = {
    // front face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    // back face
    -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
    0.5f,  0.5f,  -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

    0.5f,  0.5f,  -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
    0.5f, -0.5f,  -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

    // top face
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

    0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

    // bottom face
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

    0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,  0.0f, 0.0f,

    // right face
    0.5f, -0.5f,  -0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    0.5f,  0.5f,   0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    0.5f, -0.5f,   0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.5f,  0.5f,   0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  -0.5f,   1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    0.5f, -0.5f,  -0.5f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

    // left face
    -0.5f, -0.5f,  -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,   0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -0.5f, -0.5f,   0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    -0.5f,  0.5f,   0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  -0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f,  -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
};

float planeData[48] = {
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
    1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
};

MeshPtr meshes::CUBE;
MeshPtr meshes::PLANE;

void meshes::initialize() {
    VertexAttribSet shapeAttributeSet = {
        {GL_FLOAT, 3, sizeof(float)},
        {GL_FLOAT, 3, sizeof(float)},
        {GL_FLOAT, 2, sizeof(float)}
    };

    CUBE = std::make_unique<Mesh>(cubeVertices, 36, shapeAttributeSet);
    PLANE = std::make_unique<Mesh>(planeData, 6, shapeAttributeSet);
}