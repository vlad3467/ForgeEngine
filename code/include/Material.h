#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <memory>
#include "Texture.h"
#include <imgui.h>
#include <glm/glm.hpp>


class Material{
private:
    GLuint shaderProgram = 0;
    std::string name;

    void CheckShaderCompilation(GLuint shader);
    void CheckProgramLinking(GLuint program);

public:
    std::vector<std::shared_ptr<Texture>> textures;
    Material(const std::string& name);
    ~Material();

    GLuint CompileShaderProgram(const char* vertexShaderSource, const char* fragShaderSource);

    bool loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    void setShader(GLuint shaderProgram);
    
    void addTexture(std::shared_ptr<Texture> texture);
    void addTexture(const std::string& path);
    
    // активация
    void use();
    void bindTextures();
    
    GLuint getShaderProgram() const { return shaderProgram; }
    std::string& getName() { return name; }
    void setName(std::string nm) { name = nm; }
    
    const char* getTypeName() const { return "Material"; }
    void renderProperties();
};