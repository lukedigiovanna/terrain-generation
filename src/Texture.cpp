#include "Texture.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <iostream>

Texture::Texture(std::string const& imagePath) {
    unsigned char* data = stbi_load(imagePath.c_str(), &this->width, &this->height, &this->nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }
    glGenTextures(1, &this->texture);
    std::cout << imagePath << ": " << &this->texture << " " << this->texture << std::endl;
    glBindTexture(GL_TEXTURE_2D, this->texture);
    
    // set parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load the texture data
    GLenum format = this->nrChannels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, data);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    this->unbind();
}

Texture::~Texture() {
    // glDeleteTextures(1, &this->texture);
}

void Texture::bind() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
}

void Texture::bind(Shader const& shader, std::string const& uniformName) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    shader.setInt(uniformName.c_str(), 0);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

TexPtr textures::SMILE;
TexPtr textures::WOOD;
TexPtr textures::GRASS;
TexPtr textures::GALAXY;
TexPtr textures::MINECRAFT;

void textures::initialize() {
    SMILE = std::make_unique<Texture>("assets/smile.png");
    WOOD = std::make_unique<Texture>("assets/wood.png");
    GRASS = std::make_unique<Texture>("assets/grass.png");
    GALAXY = std::make_unique<Texture>("assets/galaxy.png");
    MINECRAFT = std::make_unique<Texture>("assets/minecraft.png");
}