#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <imgui.h>

#include "IModel.h"
#include "Material.h"
#include "Grid.h"
#include "TextRenderer.h"

class Engine {
public:
    bool Initialize(int width, int height, const char* title);
    void Run();
    void Shutdown();

private:
    // Окно и контекст
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    int m_windowWidth = 1080;
    int m_windowHeight = 720;
    bool m_running = false;

    // Ресурсы движка
    std::vector<std::unique_ptr<IModel>> m_objects;
    std::vector<std::unique_ptr<Material>> m_materials;
    
    std::unique_ptr<Grid> m_grid;
    std::unique_ptr<TextRenderer> m_textRenderer;

    // Шейдеры
    GLuint m_gridShader = 0;
    GLuint m_fpsShader = 0;

    // Выбор
    int m_selectedObject = -1;
    int m_selectedMaterial = -1;
    int m_selectedTexture = -1;

    // Шрифт по умолчанию
    std::string m_defaultFont = "../resources/fonts/lazy.ttf";

    // Камера
    glm::vec3 m_cameraPos{0.0f, 2.0f, 3.0f};
    glm::vec3 m_cameraFront{0.0f, 0.0f, -1.0f};
    glm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};
    float m_cameraSpeed = 5.0f;
    
    // Мышь
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_mouseSensitivity = 0.1f;
    bool m_cursorCaptured = false;

    // FPS
    float m_fps = 0.0f;
    float m_fpsTimer = 0.0f;
    int m_frameCount = 0;
    
    // Подсистемы
    void ProcessEvents();
    void Update(float deltaTime);
    void Render();
    void RenderUI();
    
    // Управление объектами
    IModel* AddModel(const std::string& path, const glm::vec3& pos, Material* mat);
    Material* CreateMaterial(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    void RemoveModel(int index);
    void RemoveMaterial(int index);
    void RenderRemoveTexturePopup(int texInd);
    void RenderRemoveModelPopup(int modelInd);
    void RenderRemoveMaterialPopup(int matInd);
    void RenderAddTexturePopup();
    void RemoveTextureFromMaterial(int matIndex, int texIndex);
    void RenderAll(glm::mat4& view, glm::mat4& proj);
    void RenderMaterialEditor();
    void RenderSceneEditor();
    void RenderCreateModelPopup();
    void RenderCreateMaterialPopup();
};