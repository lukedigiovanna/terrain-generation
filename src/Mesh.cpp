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