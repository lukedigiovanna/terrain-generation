#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
#include "Math.h"
#include "Terrain.h"
#include "Mesh.h"
#include "WorldObject.h"

int resizeEventWatcher(void* data, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* window = static_cast<SDL_Window*>(data);
        int scWidth, scHeight;
        SDL_GetWindowSize(window, &scWidth, &scHeight);
        glViewport(0, 0, scWidth, scHeight);
    }

    return 0;
}

int program() {
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

    meshes::initialize();
    textures::initialize();
    models::initialize();

    int maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    std::cout << maxTexSize << "\n";

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader waterShader("assets/vs.glsl", "assets/water_fs.glsl");
    Shader objectShader("assets/vs.glsl", "assets/fs.glsl");
    Shader terrainShader("assets/terrain_vs.glsl", "assets/terrain_fs.glsl");

    std::unique_ptr<Part> part = std::make_unique<Part>(Part{*meshes::CUBE, *textures::WOOD, glm::vec3(0), glm::vec3(0)});

    Terrain terrain(3284);

    std::vector<WorldObject> trees;
    for (int i = 0; i < 100; i++) {
        // gen random position
        float x = math::randf(0, 100), y = math::randf(0, 100);
        // put a tree there
        WorldObject tree = {
            models::TREE,
            glm::vec3(x,terrain.getHeight(x,y),y),
            glm::vec3(1,1,1),
            glm::vec3(0,0,0),
            0,
        }; 
        trees.push_back(tree);
    }

    bool gameActive = true;

    glm::vec3 cameraPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraForward(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraRight(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraVelocity(0.0f);
    float cameraRotation = 0.0f;
    float cameraVerticalRotation = 0.0f;

    float gravity = 10.0f;

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
                        cameraVerticalRotation = math::clampf(cameraVerticalRotation - event.motion.yrel / 250.0f, -M_PI * 0.6f, M_PI * 0.6f);
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
        float modGravity = gravity;
        if (cameraPosition.y <= 2.0f) {
            speed /= 2.0f;
            modGravity /= 6.0f;
        }
        glm::vec3 cameraForwardProjectXZ = glm::normalize(glm::vec3(cameraForward.x, 0.0f, cameraForward.z));
        if (keydown[SDLK_w]) cameraVelocity += cameraForwardProjectXZ * speed;
        if (keydown[SDLK_s]) cameraVelocity -= cameraForwardProjectXZ * speed;
        if (keydown[SDLK_a]) cameraVelocity -= cameraRight * speed;
        if (keydown[SDLK_d]) cameraVelocity += cameraRight * speed;

        // if (keydown[SDLK_SPACE]) cameraVelocity += cameraUp * speed;
        // if (keydown[SDLK_LSHIFT]) cameraVelocity -= cameraUp * speed;

        cameraVelocity.y -= modGravity * dt;
        cameraPosition += cameraVelocity * dt;

        float height = terrain.getHeight(cameraPosition.x, cameraPosition.z);

        if (cameraPosition.y <= height + 2.0f) {
            cameraVelocity.y = 0;
            cameraPosition.y = height + 2.0f;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        int scWidth, scHeight;
        SDL_GetWindowSize(window, &scWidth, &scHeight);

        glm::mat4 proj = glm::perspective(90.0f, scWidth / static_cast<float>(scHeight), 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraForward, glm::vec3(0.0f, 1.0f, 0.0f));

        
        glm::vec3 lightPos = cameraPosition + glm::vec3(0, 10, 0);

        terrainShader.use();
        terrainShader.setVec3("lightPosition", lightPos);
        terrainShader.setMatrix4("projection", proj);
        terrainShader.setMatrix4("view", view);
        terrainShader.setVec2("spriteSheetSize", 24, 34);
        // terrain
        textures::MINECRAFT->bind();
        terrain.render(objectShader, cameraPosition.x, cameraPosition.z);
        
        objectShader.use();

        objectShader.setVec3("lightPosition", lightPos);
        objectShader.setMatrix4("projection", proj);
        objectShader.setMatrix4("view", view);
        objectShader.setMatrix4("model", glm::mat4(1.0f));

        // render the tree model
        for (const auto & tree : trees) {
            for (const auto & part : tree.model) {
                glm::mat4 model(1.0f);
                model = glm::translate(model, tree.pos + part->offsetPosition);
                model = glm::scale(model, tree.scale + part->scale);
                objectShader.setMatrix4("model", model);
                part->texture.bind();
                part->mesh.render();
            };
        }

        // skybox
        glm::mat4 model(1.0f);
        model = glm::translate(model, cameraPosition);
        model = glm::scale(model, glm::vec3(250.0f, 250.0f, 250.0f));
        objectShader.setMatrix4("model", model);
        textures::GALAXY->bind();
        meshes::CUBE->render();

        // water
        waterShader.use();
        waterShader.setMatrix4("projection", proj);
        waterShader.setMatrix4("view", view);
        int waterSize = 200;
        waterShader.setMatrix4("model", glm::scale(
            glm::translate(
            glm::mat4(1.0f),
            glm::vec3(cameraPosition.x - waterSize / 2, 0, cameraPosition.z - waterSize / 2)), 
            glm::vec3(waterSize, 1, waterSize)
        ));

        waterShader.setFloat("t", currTime);
        meshes::PLANE->render();

        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();

    return 0;
}

#ifdef _WIN32
#include <windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return program();    
}
#else
int main() {
    return program();
}
#endif