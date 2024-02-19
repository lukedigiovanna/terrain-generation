#pragma once

class Mesh {
private:
    unsigned int vao;
    unsigned int numVertices;
public:
    Mesh(float* data, int numVertices);

    void render() const;
};