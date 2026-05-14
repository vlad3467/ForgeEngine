#pragma once

#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include <IModel.h>
#include <ObjParser.h>

class Model : public IModel {

private:
    std::string typeName = "Model";
    GLuint VAO, VBO, EBO;
    Material* material;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 position{0,0,0};
    glm::vec4 rotation{0, 1, 0,-90};
    

public:
    void Render(glm::mat4& view, glm::mat4& proj) override;
    bool LoadObjModel(std::string& pathImage);
    void RenderProperties() override;
    ~Model() override;
    glm::vec3 GetPosition() override { return position; };
    glm::vec4 GetRotation() { return rotation; };
    std::string& GetTypeName() override { return typeName; }
    void SetPosition(glm::vec3 newPos){ position = newPos; };
    void SetRotation(glm::vec4 newRot){ rotation = newRot; };
    void SetMaterial(Material* mat) override { material = mat; };
    Material* GetMaterial() override { return material; }
};