#pragma once

#include "Shader.h"

#include <string>

class Texture {
private:
    int width, height, nrChannels;
    unsigned int texture;
public:
    Texture(std::string const& imagePath);
    ~Texture();

    void bind() const;
    void bind(Shader const& shader, std::string const& uniformName) const;
    void unbind() const;
};


using TexPtr = std::unique_ptr<Texture>;

namespace textures {
    extern TexPtr SMILE;
    extern TexPtr WOOD;
    extern TexPtr GRASS;
    extern TexPtr GALAXY;
    extern TexPtr MINECRAFT;

    void initialize();
}