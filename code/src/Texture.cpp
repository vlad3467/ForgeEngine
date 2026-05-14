#include "Texture.h"
#include <iostream>
#include <glad/glad.h>

Texture::Texture(const std::string& pathImage) 
    : texture(0), width(0), height(0), initialized(false) 
{
    SDL_Surface* image = IMG_Load(pathImage.c_str());
    if (!image) {
        std::cout << "ERROR::TEXTURE: Failed to load texture: " << pathImage << " - " << SDL_GetError() << "\n";
        return;
    }
    
    width = image->w;
    height = image->h;
    
    GLenum format = GL_RGBA;
    if (image->format == SDL_PIXELFORMAT_RGB24) {
        format = GL_RGB;
    }
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, 
                 image->w, image->h, 0, 
                 format, GL_UNSIGNED_BYTE, image->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Texture loaded: " << pathImage << " ID: " << texture << std::endl;
    
    SDL_DestroySurface(image);
    
    initialized = true;
}

Texture::~Texture() {
    if (texture) {
        glDeleteTextures(1, &texture);
    }
}

void Texture::Bind() const {
    glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}