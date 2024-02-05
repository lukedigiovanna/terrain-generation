#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <windows.h>

#include <stdexcept>
#include <iostream>

#include <vector>
#include <unordered_map>

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"

float randf(float a, float b) {
    float r = (rand() % 10000) / 10000.0f;
    return r * (b - a) + a;
}

inline float absf(float f) {
    return f < 0 ? -f : f;
}

inline float clampf(float f, float a, float b) {
    return f < a ? a : (f > b ? b : f);
}

inline float interpolate(float a0, float a1, float w) {
    return (a1 - a0) * w + a0;
}

glm::vec2 randomGradient(int ix, int iy, unsigned seed=6482) {
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    // unsigned a = hash(ix + seed), b = hash(iy + hash(seed));
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= a << s | a >> w-s;
    b *= 1911520717; a ^= b << s | b >> w-s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    glm::vec2 v{std::cos(random), std::sin(random)};
    return v;
}

float dotGridGradient(int ix, int iy, float x, float y, unsigned seed=53842) {
    glm::vec2 gradient = randomGradient(ix, iy, seed);
    glm::vec2 d = {
        x - static_cast<float>(ix), 
        y - static_cast<float>(iy)
    };
    return glm::dot(d, gradient);
}

float getPerlinNoise(float x, float y) {
    int x0 = static_cast<int>(x); 
    int x1 = x0 + 1;
    int y0 = static_cast<int>(y); 
    int y1 = y0 + 1;

    float sx = x - static_cast<float>(x0);
    float sy = y - static_cast<float>(y0);

    float n0, n1;
    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);

    float ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    
    float ix1 = interpolate(n0, n1, sx);

    return interpolate(ix0, ix1, sy);
}

int resizeEventWatcher(void* data, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* window = static_cast<SDL_Window*>(data);
        int scWidth, scHeight;
        SDL_GetWindowSize(window, &scWidth, &scHeight);
        glViewport(0, 0, scWidth, scHeight);
    }

    return 0;
}

struct Ball {
    glm::vec3 pos;
    glm::vec3 velocity;
    float angle;
};

struct Mesh {
    unsigned int vao;
    unsigned int numVertices;

    void render() {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
    }
};

void loadMesh(Mesh& mesh, float* data, int numVertices) {
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(mesh.vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * numVertices, data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    mesh.numVertices = numVertices;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    stbi_set_flip_vertically_on_load(true);  
    
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // For example, OpenGL 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Evolution", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    SDL_AddEventWatch(resizeEventWatcher, window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader waterShader("assets/vs.glsl", "assets/water_fs.glsl");
    Shader objectShader("assets/vs.glsl", "assets/fs.glsl");

    Texture smile("assets/smile.png");
    Texture grass("assets/grass.png");
    Texture galaxy("assets/galaxy.png");

    float cube[288] = {
        // front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,

        // back face
        -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

         0.5f,  0.5f,  -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

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
    Mesh squareMesh;
    loadMesh(squareMesh, cube, 36);

    float planeData[48] = {
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    };
    Mesh plane;
    loadMesh(plane, planeData, 6);

    const int terrainSize = 100;
    float latticePoints[terrainSize + 1][terrainSize + 1];
    for (int i = 0; i < terrainSize + 1; i++) {
        for (int j = 0; j < terrainSize + 1; j++) {
            float s = 18.45231f;
            float f = getPerlinNoise(i / s, j / s);
            latticePoints[i][j] = f * f * 52 - 2;
            // latticePoints[i][j] = f;
        }
    }
    float terrainData[terrainSize * terrainSize * 48];
    int ti = 0;
    for (int i = 0; i < terrainSize; i++) {
        for (int j = 0; j < terrainSize; j++) {
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
    Mesh terrainMesh;
    loadMesh(terrainMesh, terrainData, terrainSize * terrainSize* 6);
    
    bool gameActive = true;

    glm::vec3 cameraPosition(25.0f, 0.0f, 25.0f);
    glm::vec3 cameraForward(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraRight(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraVelocity(0.0f);
    float cameraRotation = 0.0f;
    float cameraVerticalRotation = 0.0f;

    float gravity = 10.0f;

    std::vector<Ball> balls;

    size_t n = 25;
    for (int i = 0; i < n; i++) {
        float p = i / static_cast<float>(n);
        float ang = i / static_cast<float>(n) * M_PI * 2;
        balls.push_back({
            glm::vec3(0.0f),
            glm::vec3(cosf(ang), sinf(ang), p - 0.5f),
            0.0f
        });
    }

    float lastTime = static_cast<float>(SDL_GetTicks()) / 1000.0f;

    std::unordered_map<int, bool> keydown;

    bool cursorLocked = false;

    while (gameActive) {
        float currTime = static_cast<float>(SDL_GetTicks()) / 1000.0f;
        float dt = currTime - lastTime;
        lastTime = currTime;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    gameActive = false;
                    break;
                case SDL_KEYDOWN:
                    keydown[event.key.keysym.sym] = true;
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            cursorLocked = false;
                            break;
                        case SDLK_SPACE:
                            cameraVelocity.y = 5.0f;
                    }
                    break;
                case SDL_KEYUP:
                    keydown[event.key.keysym.sym] = false;
                    break;
                case SDL_MOUSEMOTION:
                    if (cursorLocked) {
                        cameraRotation -= event.motion.xrel / 250.0f;
                        cameraVerticalRotation = clampf(cameraVerticalRotation - event.motion.yrel / 250.0f, -M_PI * 0.6f, M_PI * 0.6f);
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    cursorLocked = true;
                    break;
            }
        }

        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(cursorLocked)); 

        cameraForward = glm::vec3(-sinf(cameraRotation), sinf(cameraVerticalRotation), -cosf(cameraRotation));
        cameraRight = glm::vec3(cosf(cameraRotation), 0.0f, -sinf(cameraRotation));
        glm::vec3 cameraUp = glm::cross(cameraRight, cameraForward);
        cameraVelocity = glm::vec3(0.0f, cameraVelocity.y, 0.0f);
        float speed = 5.0f;
        glm::vec3 cameraForwardProjectXZ = glm::normalize(glm::vec3(cameraForward.x, 0.0f, cameraForward.z));
        if (keydown[SDLK_w]) cameraVelocity += cameraForwardProjectXZ * speed;
        if (keydown[SDLK_s]) cameraVelocity -= cameraForwardProjectXZ * speed;
        if (keydown[SDLK_a]) cameraVelocity -= cameraRight * speed;
        if (keydown[SDLK_d]) cameraVelocity += cameraRight * speed;

        // if (keydown[SDLK_SPACE]) cameraVelocity += cameraUp * speed;
        // if (keydown[SDLK_LSHIFT]) cameraVelocity -= cameraUp * speed;

        cameraVelocity.y -= gravity * dt;
        cameraPosition += cameraVelocity * dt;

        if (cameraPosition.y <= 2.0f) {
            cameraVelocity.y = 0;
            cameraPosition.y = 2;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        int scWidth, scHeight;
        SDL_GetWindowSize(window, &scWidth, &scHeight);

        glm::mat4 proj = glm::perspective(90.0f, scWidth / static_cast<float>(scHeight), 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraForward, glm::vec3(0.0f, 1.0f, 0.0f));

        objectShader.use();

        objectShader.setVec3("lightPosition", cameraPosition);

        objectShader.setMatrix4("projection", proj);
        objectShader.setMatrix4("view", view);
        objectShader.setMatrix4("model", glm::mat4(1.0f));

        grass.bind();

        terrainMesh.render();

        // smile.bind();
        for (int i = 0; i < n; i++) {
            Ball& ball = balls[i];
            // ball.pos += ball.velocity * dt;
            float p = static_cast<float>(i) / n * 2 * M_PI + currTime / 10;
            ball.pos = glm::vec3(cosf(p) * 20, 0.0f, sinf(p) * 20);
            ball.angle += dt;
            if (absf(ball.pos.x) >= 2.5f) ball.velocity.x *= -1;
            if (absf(ball.pos.y) >= 2.5f) ball.velocity.y *= -1;
            if (absf(ball.pos.z) >= 2.5f) ball.velocity.z *= -1;
            glm::mat4 model(1.0f);
            model = glm::translate(model, ball.pos + glm::vec3(25.0f, 5.0f, 25.0f));
            model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
            model = glm::rotate(model, ball.angle, glm::vec3(0.0f, 1.0f, 0.0f));
            objectShader.setMatrix4("model", model);
            squareMesh.render();
        }


        // skybox
        glm::mat4 model(1.0f);
        model = glm::translate(model, cameraPosition);
        model = glm::scale(model, glm::vec3(250.0f, 250.0f, 250.0f));
        objectShader.setMatrix4("model", model);
        galaxy.bind();
        squareMesh.render();

        // water
        waterShader.use();
        waterShader.setMatrix4("projection", proj);
        waterShader.setMatrix4("view", view);
        waterShader.setMatrix4("model", glm::scale(glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, -0.5f, 0.0f)), glm::vec3(100, 1, 100)));

        waterShader.setFloat("t", currTime);
        plane.render();

        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}