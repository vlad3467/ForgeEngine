#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include <IModel.h>

class Grid : public IModel {
private:
    std::string typeName = "Grid";
    GLuint VAO = 0, VBO = 0;
    Material* material = 0;
    int vertexCount = 0;
    
    int size;       
    float step;     
    glm::vec3 color{0.5f, 0.5f, 0.5f};
    float opacity = 0.5f;
    glm::vec3 position;

    void GenerateGrid();

public:
    Grid(int size = 10, float step = 1.0f);
    ~Grid() override;

    void Render(glm::mat4& view, glm::mat4& projection) override;
    std::string& GetTypeName() override { return typeName; }
    void RenderProperties() override;

    glm::vec3 GetPosition() override { return position; }
    Material* GetMaterial() override { return material; }
    void SetMaterial(Material* mat) override { material = mat; }
    
    void SetColor(const glm::vec3& col) { color = col; }
    void SetOpacity(float opa) { opacity = opa; }
};