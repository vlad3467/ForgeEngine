#pragma once
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <string>

class Texture {
private:
    GLuint texture;
    int width;
    int height;
    bool initialized;

public:
    Texture(const std::string& pathImage);
    ~Texture();
    
    bool IsLoaded() const { return initialized; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    GLuint GetTexture() const { return texture; }
    
    void Bind() const;
    void Unbind() const;
};