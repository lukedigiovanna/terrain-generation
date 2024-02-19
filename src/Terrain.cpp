#include "Terrain.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Math.h"

TerrainCell::TerrainCell(int x, int z, int seed) {
    for (int i = 0; i < TERRAIN_CELL_SIZE + 1; i++) {
        for (int j = 0; j < TERRAIN_CELL_SIZE + 1; j++) {
            float s = 18.45231f;
            float f = math::getPerlinNoise((x * TERRAIN_CELL_SIZE + i) / s, (z * TERRAIN_CELL_SIZE + j) / s, seed);
            latticePoints[i][j] = f * f * 52 - 2;
            std::cout << f << "\n";
            // latticePoints[i][j] = f;
        }
    }
    float terrainData[TERRAIN_CELL_SIZE * TERRAIN_CELL_SIZE * 48];
    int ti = 0;
    for (int i = 0; i < TERRAIN_CELL_SIZE; i++) {
        for (int j = 0; j < TERRAIN_CELL_SIZE; j++) {
            float s = 1.0f;
            float fi = static_cast<float>(i) * s, fj = static_cast<float>(j) * s;
            glm::vec3 diag(1.0f, latticePoints[i + 1][j + 1] - latticePoints[i][j], 1.0f);
            glm::vec3 right(1.0f, latticePoints[i + 1][j] - latticePoints[i][j], 0.0f);
            glm::vec3 up(0.0f, latticePoints[i][j + 1] - latticePoints[i][j], 1.0f);
            glm::vec3 norm1 = glm::normalize(glm::cross(diag, right));
            if (norm1.y < 0) norm1 = -norm1;
            glm::vec3 norm2 = glm::normalize(glm::cross(diag, up));
            if (norm2.y < 0) norm2 = -norm2;
            // std::cout << norm1.x << ", " << norm1.y << ", " << norm1.z << "\n";
            float triangles[48] = {
                fi,     latticePoints[i][j],         fj,      norm1.x, norm1.y, norm1.z,  0.0f, 0.0f,
                fi + s, latticePoints[i + 1][j],     fj,      norm1.x, norm1.y, norm1.z,  1.0f, 0.0f,
                fi + s, latticePoints[i + 1][j + 1], fj + s,  norm1.x, norm1.y, norm1.z,  1.0f, 1.0f,

                fi,     latticePoints[i][j],         fj,      norm2.x, norm2.y, norm2.z,  0.0f, 0.0f,
                fi,     latticePoints[i][j + 1],     fj + s,  norm2.x, norm2.y, norm2.z,  0.0f, 1.0f,
                fi + s, latticePoints[i + 1][j + 1], fj + s,  norm2.x, norm2.y, norm2.z,  1.0f, 1.0f,
            };
            for (int i = 0; i < 48; i++) {
                terrainData[ti + i] = triangles[i];
            }
            ti += 48;
        }
    }
    mesh = std::make_unique<Mesh>(terrainData, TERRAIN_CELL_SIZE * TERRAIN_CELL_SIZE * 6);
}

Mesh& TerrainCell::getMesh() {
    return *mesh;
}

Terrain::Terrain(int seed) : seed(seed) {

}

void Terrain::render(Shader& shader, float x, float z) {
    int cellX = static_cast<int>(x / TERRAIN_CELL_SIZE);
    int cellZ = static_cast<int>(z / TERRAIN_CELL_SIZE);
    for (int cx = cellX - TERRAIN_RENDER_DISTANCE; cx <= cellX + TERRAIN_RENDER_DISTANCE; cx++) {
        for (int cz = cellZ - TERRAIN_RENDER_DISTANCE; cz <= cellZ + TERRAIN_RENDER_DISTANCE; cz++) {
            // check if this cell is in the cache
            int hash = cx * 32768 + cz;
            if (cells.find(hash) == cells.end()) {
                // need to generate this terrain cell then
                cells.insert_or_assign(hash, std::make_unique<TerrainCell>(cx, cz, seed));
            }
            TerrainCell& tcell = *cells[hash];
            // setup model matrix
            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(cx * TERRAIN_CELL_SIZE, 0, cz * TERRAIN_CELL_SIZE));
            shader.setMatrix4("model", model);
            // render 
            tcell.getMesh().render();
        }
    }   
}