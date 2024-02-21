#pragma once

#include "Shader.h"
#include "Mesh.h"

#include <unordered_map>
#include <memory>

#define TERRAIN_RESOLUTION 0.5f // The ammount of world space between each lattice point
#define TERRAIN_CELL_SIZE 10
#define TERRAIN_RENDER_DISTANCE 10
#define TERRAIN_CACHE_CAPACITY 25

class TerrainCell {
private:
    int x, z;
    float latticePoints[TERRAIN_CELL_SIZE + 1][TERRAIN_CELL_SIZE + 1];
    std::unique_ptr<Mesh> mesh;
public:
    // calling the constructor will generate the mesh for this terrain cell
    TerrainCell(int x, int z, int seed);

    float getHeight(float x, float z) const;
    Mesh& getMesh();
};

class Terrain {
private:
    // should implemented as an LRU
    std::unordered_map<int, std::unique_ptr<TerrainCell>> cells;
    int seed;
public:
    Terrain(int seed);

    // Queries the terrain cells to find the precise height of the terrain at the given x,z coordinate
    float getHeight(float x, float z);
    // Given some x, z we will render the surrounding cells in their proper place
    void render(Shader& shader, float x, float z);
};
