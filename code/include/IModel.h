#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>

#include <IModel.h>
#include <Material.h>

class IModel {
public:
    virtual ~IModel() = default;
    
    // Общие методы для всех объектов
    virtual void Render(glm::mat4& view, glm::mat4& proj) = 0;
    virtual std::string& GetTypeName() = 0;
    virtual glm::vec3 GetPosition() = 0;
    virtual Material* GetMaterial() = 0;
    virtual void SetMaterial(Material* mat) = 0;
    
    // Для ImGui редактирования
    virtual void RenderProperties() {}
};
