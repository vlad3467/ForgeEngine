#include "Material.h"
#include <iostream>

void Material::CheckShaderCompilation(GLuint shader){
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER: Compilation failed \n" << infoLog << std::endl;
    }
}

void Material::CheckProgramLinking(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM: Linking failed\n" 
                  << infoLog << std::endl;
    }
}

GLuint Material::CompileShaderProgram(const char* vertexShaderSource, const char* fragShaderSource){
    size_t vertexFileSize;
    char* vertexShaderFile = (char*)SDL_LoadFile(vertexShaderSource, &vertexFileSize);
    if (!vertexShaderFile) {
        std::cerr << "ERROR::SHADER: Failed to load vertex shader file: " << vertexShaderSource << std::endl;
        return 0;
    }

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderFile, NULL);
    glCompileShader(vertexShader);
    CheckShaderCompilation(vertexShader);
            
    size_t fragFileSize;
    char* fragShaderFile = (char*)SDL_LoadFile(fragShaderSource, &fragFileSize);
    if (!fragShaderFile) {
        std::cerr << "ERROR::SHADER: Failed to load fragment shader file: " << fragShaderSource << std::endl;
        return 0;
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderFile, NULL);
    glCompileShader(fragShader);
    CheckShaderCompilation(fragShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    CheckProgramLinking(shaderProgram);

    SDL_free(vertexShaderFile);
    SDL_free(fragShaderFile);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    return shaderProgram;
}

Material::Material(const std::string& name) : name(name) {}

Material::~Material() {
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
    }
}

bool Material::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    shaderProgram = CompileShaderProgram(vertexPath.c_str(), fragmentPath.c_str());
    return shaderProgram != 0;
}

void Material::setShader(GLuint shaderProg) {
    shaderProgram = shaderProg;
}

void Material::addTexture(std::shared_ptr<Texture> texture) {
    textures.push_back(texture);
}

void Material::addTexture(const std::string& path) {
    textures.push_back(std::make_shared<Texture>(path));
}

void Material::use() {
    glUseProgram(shaderProgram);
}

void Material::bindTextures() {
    for (size_t i = 0; i < textures.size() && i < 16; i++) {  // Максимум 16 текстур
        glActiveTexture(GL_TEXTURE0 + i);
        textures[i]->Bind();
        
        std::string uniformName = "texture" + std::to_string(i);
        GLint loc = glGetUniformLocation(shaderProgram, uniformName.c_str());
        if (loc != -1) {
            glUniform1i(loc, i);
        }
        else {
            std::cout << "WARNING: Uniform '" << uniformName << "' not found in shader " << shaderProgram << std::endl;
        }
    }
}

void Material::renderProperties() {
    ImGui::Text("Material: %s", name.c_str());
    ImGui::Text("Shader Program: %u", shaderProgram);
    ImGui::Text("Textures: %zu", textures.size());
    
    for (size_t i = 0; i < textures.size(); i++) {
        ImGui::Text("  Texture %zu: %dx%d", i, 
                    textures[i]->GetWidth(), 
                    textures[i]->GetHeight());
    }
}