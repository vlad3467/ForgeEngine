#pragma once
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <string>
#include <map>
#include <glm/glm.hpp>

class TextRenderer{
private:

    GLuint VBO, VAO;

    struct Character {
        GLuint texture;
        glm::ivec2   size;
        glm::ivec2   bearing;
        long advance;
    };

    std::map<char, Character> Characters;

public:
    TextRenderer(const std::string& pathFont);
    ~TextRenderer();
    
    void RenderText(GLuint shaderProgram, glm::mat4& proj, std::string text, float x, float y, float scale, glm::vec3 color);
};