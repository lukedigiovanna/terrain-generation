#include "Terrain.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include "Math.h"

#include "Mesh.h"
#include "Shader.h"

static const VertexAttribSet terrainAttributeSet = {
    {GL_FLOAT, 3, sizeof(float)},
    {GL_FLOAT, 3, sizeof(float)},
    {GL_FLOAT, 2, sizeof(float)},
    {GL_FLOAT, 1, sizeof(float)}
};

enum TextureID {
    GRASS=0,
    STONE=1,
    DIRT=2,
    COBBLESTONE=24,
    SAND=26,
    GRAVEL=27
};

static float getTexture(float height) {
    if (height < 0) return SAND;
    else if (height < 2) return STONE;
    else if (height < 4) return DIRT;
    else return GRASS;
}

TerrainCell::TerrainCell(int x, int z, int seed) : x(x), z(z) {
    for (int i = 0; i < TERRAIN_POINTS_PER_CELL + 1; i++) {
        for (int j = 0; j < TERRAIN_POINTS_PER_CELL + 1; j++) {
            float s = 43.45231f;
            float f = math::getPerlinNoise(
                0.4837f + (x * TERRAIN_CELL_SIZE + static_cast<float>(i) / TERRAIN_RESOLUTION) / s, 
                0.9482f + (z * TERRAIN_CELL_SIZE + static_cast<float>(j) / TERRAIN_RESOLUTION) / s, 
                seed
            );
            latticePoints[i][j] = f * f * 52 - 2;
        }
    }
    const int floatsPerLatticeCell = 54;
    float terrainData[TERRAIN_POINTS_PER_CELL * TERRAIN_POINTS_PER_CELL * floatsPerLatticeCell];
    int ti = 0;
    for (int i = 0; i < TERRAIN_POINTS_PER_CELL; i++) {
        for (int j = 0; j < TERRAIN_POINTS_PER_CELL; j++) {
            float s = 1.0f / static_cast<float>(TERRAIN_RESOLUTION);
            float fi = static_cast<float>(i) * s, fj = static_cast<float>(j) * s;
            glm::vec3 diag(1.0f, latticePoints[i + 1][j + 1] - latticePoints[i][j], 1.0f);
            glm::vec3 right(1.0f, latticePoints[i + 1][j] - latticePoints[i][j], 0.0f);
            glm::vec3 up(0.0f, latticePoints[i][j + 1] - latticePoints[i][j], 1.0f);
            glm::vec3 norm1 = glm::normalize(glm::cross(diag, right));
            if (norm1.y < 0) norm1 = -norm1;
            glm::vec3 norm2 = glm::normalize(glm::cross(diag, up));
            if (norm2.y < 0) norm2 = -norm2;
            // std::cout << norm1.x << ", " << norm1.y << ", " << norm1.z << "\n";
            float wi = fi - i / TERRAIN_RESOLUTION;
            float wj = fj - j / TERRAIN_RESOLUTION;
            float triangles[floatsPerLatticeCell] = {
                fi,     latticePoints[i][j],         fj,      norm1.x, norm1.y, norm1.z,  wi,     wj,       getTexture(latticePoints[i][j]),        //getTexture(latticePoints[i][j]),//
                fi + s, latticePoints[i + 1][j],     fj,      norm1.x, norm1.y, norm1.z,  wi + s, wj,       getTexture(latticePoints[i + 1][j]),    //getTexture(latticePoints[i][j]),//
                fi + s, latticePoints[i + 1][j + 1], fj + s,  norm1.x, norm1.y, norm1.z,  wi + s, wj + s,   getTexture(latticePoints[i + 1][j + 1]),//getTexture(latticePoints[i][j]),//
                fi,     latticePoints[i][j],         fj,      norm2.x, norm2.y, norm2.z,  wi,     wj,       getTexture(latticePoints[i][j]),        //getTexture(latticePoints[i][j]),//
                fi,     latticePoints[i][j + 1],     fj + s,  norm2.x, norm2.y, norm2.z,  wi,     wj + s,   getTexture(latticePoints[i][j + 1]),    //getTexture(latticePoints[i][j]),//
                fi + s, latticePoints[i + 1][j + 1], fj + s,  norm2.x, norm2.y, norm2.z,  wi + s, wj + s,   getTexture(latticePoints[i + 1][j + 1]),//getTexture(latticePoints[i][j]),//
            };
            for (int i = 0; i < floatsPerLatticeCell; i++) {
                terrainData[ti + i] = triangles[i];
            }
            ti += floatsPerLatticeCell;
        }
    }
    mesh = std::make_unique<Mesh>(terrainData, TERRAIN_POINTS_PER_CELL * TERRAIN_POINTS_PER_CELL * 6, terrainAttributeSet);

    // populate with trees
    for (int i = 0; i < 2; i++) {
        float wx = math::randf(
            x * TERRAIN_CELL_SIZE, 
            (x + 1) * TERRAIN_CELL_SIZE), 
        wz = math::randf(z * TERRAIN_CELL_SIZE, (z + 1) * TERRAIN_CELL_SIZE);
        // put a tree there
        float h = getHeight(wx, wz);
        if (h < 1.0f) {
            continue;
        }
        WorldObject tree = {
            models::TREE,
            glm::vec3(wx,getHeight(wx,wz),wz),
            glm::vec3(1,1,1),
            glm::vec3(0,0,0),
            0,
        }; 
        objects.push_back(tree);
    }
}

float TerrainCell::getHeight(float x, float z) const {
    float px = x - static_cast<float>(this->x * TERRAIN_CELL_SIZE);
    float pz = z - static_cast<float>(this->z * TERRAIN_CELL_SIZE);
    if (px < 0 || px >= TERRAIN_CELL_SIZE || pz < 0 || pz >= TERRAIN_CELL_SIZE) {
        throw std::runtime_error("TerrainCell::getHeight: Coordinate out of range of this terrain cell (querying: "
                                     + std::to_string(x) + ", " + std::to_string(z) + " in terrain cell: " + std::to_string(this->x) + ", " + std::to_string(this->z) + ")");
    }
    int x0 = static_cast<int>(px * TERRAIN_RESOLUTION);
    int x1 = x0 + 1;
    int z0 = static_cast<int>(pz * TERRAIN_RESOLUTION);
    int z1 = z0 + 1;
    float h00 = latticePoints[x0][z0];
    float h01 = latticePoints[x0][z1];
    float h10 = latticePoints[x1][z0];
    float h11 = latticePoints[x1][z1];
    return (h00 + h01 + h10 + h11) / 4;
}

Mesh& TerrainCell::getMesh() {
    return *mesh;
}

void TerrainCell::render(Shader& terrainShader, Shader& objectShader) const {
    // render terrain mesh
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(x * TERRAIN_CELL_SIZE, 0, z * TERRAIN_CELL_SIZE));
    terrainShader.use();
    terrainShader.setMatrix4("model", model);
    textures::MINECRAFT->bind();
    mesh->render();
    // render world objects
    for (auto const& object : objects) {
        object.render(objectShader);
    }
}

Terrain::Terrain(int seed) : seed(seed) {

}

#define TERRAIN_HASH(cx, cz) (cx * 32768 + cz)

void Terrain::render(Shader& terrainShader, Shader& objectShader, float x, float z) {
    int cellX = static_cast<int>(x / TERRAIN_CELL_SIZE);
    int cellZ = static_cast<int>(z / TERRAIN_CELL_SIZE);
    for (int cx = cellX - TERRAIN_RENDER_DISTANCE; cx <= cellX + TERRAIN_RENDER_DISTANCE; cx++) {
        for (int cz = cellZ - TERRAIN_RENDER_DISTANCE; cz <= cellZ + TERRAIN_RENDER_DISTANCE; cz++) {
            // check if this cell is in the cache
            int hash = TERRAIN_HASH(cx, cz);
            if (cells.find(hash) == cells.end()) {
                // need to generate this terrain cell then
                cells.insert_or_assign(hash, std::make_unique<TerrainCell>(cx, cz, seed));
            }
            TerrainCell& tcell = *cells[hash];
            tcell.render(terrainShader, objectShader);
        }
    }  
}

float Terrain::getHeight(float x, float z) {
    int cellX = static_cast<int>(x / TERRAIN_CELL_SIZE);
    int cellZ = static_cast<int>(z / TERRAIN_CELL_SIZE);
    // see if this cell exists
    int hash = TERRAIN_HASH(cellX, cellZ);
    if (cells.find(hash) == cells.end()) {
        cells.insert_or_assign(hash, std::make_unique<TerrainCell>(cellX, cellZ, seed));
    }
    TerrainCell& cell = *cells[hash];
    return cell.getHeight(x, z);
}