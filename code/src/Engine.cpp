#include "Engine.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <iostream>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "Model.h"

// ========================================
// Инициализация
// ========================================
bool Engine::Initialize(int width, int height, const char* title) {
    m_windowWidth = width;
    m_windowHeight = height;

    // SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(title, width, height, 
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        SDL_Log("OpenGL context failed: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("GLAD init failed");
        return false;
    }

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); 

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    ImGui::GetStyle().ScaleAllSizes(scale);
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext)) {
        SDL_Log("ImGui SDL3 backend failed");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        SDL_Log("ImGui OpenGL3 backend failed");
        return false;
    }

    m_textRenderer = std::make_unique<TextRenderer>(m_defaultFont);

    // Шейдеры
    Material* subsidiaryMaterial = CreateMaterial("For grid", "../code/shaders/vertexShaders/grid.vert", "../code/shaders/fragmentShaders/grid.frag");

    m_gridShader = subsidiaryMaterial->loadShader(
        "../code/shaders/vertexShaders/grid.vert",
        "../code/shaders/fragmentShaders/grid.frag");
    
    m_fpsShader = subsidiaryMaterial->CompileShaderProgram(
        "../code/shaders/vertexShaders/fpsText.vert",
        "../code/shaders/fragmentShaders/fpsText.frag");

    m_grid = std::make_unique<Grid>(100, 1);
    m_grid->SetMaterial(subsidiaryMaterial);

    // Инициализация камеры
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(direction);

    std::cout << "Engine initialized successfully!\n";
    return true;
}

// ========================================
// Главный цикл
// ========================================
void Engine::Run() {
    m_running = true;
    float lastTime = SDL_GetTicks() / 1000.0f;

    while (m_running) {
        float currentTime = SDL_GetTicks() / 1000.0f;
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        if (deltaTime > 0.1f) deltaTime = 0.016f;

        ProcessEvents();
        Update(deltaTime);
        Render();
        RenderUI();

        SDL_GL_SwapWindow(m_window);
    }
}

// ========================================
// События
// ========================================
void Engine::ProcessEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT) m_running = false;
        
        if (e.type == SDL_EVENT_WINDOW_RESIZED) {
            m_windowWidth = e.window.data1;
            m_windowHeight = e.window.data2;
            glViewport(0, 0, m_windowWidth, m_windowHeight);
        }

        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (e.button.button == SDL_BUTTON_RIGHT && !ImGui::GetIO().WantCaptureMouse) {
                const bool* keys = SDL_GetKeyboardState(NULL);
                if (keys[SDL_SCANCODE_LCTRL]) {
                    m_cursorCaptured = true;
                    SDL_SetWindowRelativeMouseMode(m_window, true);
                }
            }
        }

        if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (e.button.button == SDL_BUTTON_RIGHT) {
                m_cursorCaptured = false;
                SDL_SetWindowRelativeMouseMode(m_window, false);
            }
        }

        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_LCTRL && m_cursorCaptured) {
            m_cursorCaptured = false;
            SDL_SetWindowRelativeMouseMode(m_window, false);
        }

        if (e.type == SDL_EVENT_MOUSE_MOTION && m_cursorCaptured && !ImGui::GetIO().WantCaptureMouse) {
            m_yaw += e.motion.xrel * m_mouseSensitivity;
            m_pitch -= e.motion.yrel * m_mouseSensitivity;
            
            if (m_pitch > 89.0f) m_pitch = 89.0f;
            if (m_pitch < -89.0f) m_pitch = -89.0f;

            glm::vec3 dir;
            dir.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            dir.y = sin(glm::radians(m_pitch));
            dir.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_cameraFront = glm::normalize(dir);
        }
    }
}

// ========================================
// Обновление
// ========================================
void Engine::Update(float deltaTime) {
    // FPS
    m_fpsTimer += deltaTime;
    m_frameCount++;
    if (m_fpsTimer >= 0.1f) {
        m_fps = m_frameCount / m_fpsTimer;
        m_fpsTimer = 0.0f;
        m_frameCount = 0;
    }

    // Клавиатура
    const bool* keys = SDL_GetKeyboardState(NULL);
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        float speed = m_cameraSpeed * deltaTime;
        if (keys[SDL_SCANCODE_W]) m_cameraPos += m_cameraFront * speed;
        if (keys[SDL_SCANCODE_S]) m_cameraPos -= m_cameraFront * speed;
        if (keys[SDL_SCANCODE_A]) 
            m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;
        if (keys[SDL_SCANCODE_D]) 
            m_cameraPos += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;
    }
}

// ========================================
// Рендеринг
// ========================================
void Engine::Render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                      (float)m_windowWidth / m_windowHeight,
                                      0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // Грид
    if (m_grid) m_grid->Render(view, proj);

    // Все модели
    RenderAll(view, proj);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // FPS
    glm::mat4 fpsProj = glm::ortho(0.0f, (float)m_windowWidth, (float)m_windowHeight, 0.0f);
    std::string fpsText = "FPS: " + std::to_string((int)m_fps);
    if (m_textRenderer) 
        m_textRenderer->RenderText(m_fpsShader, fpsProj, fpsText, 10, 10, 0.8f, glm::vec3(0.5f, 0.8f, 0.2f));
}

// ========================================
// Управление объектами
// ========================================
Material* Engine::CreateMaterial(const std::string& name,
                                 const std::string& vertPath,
                                 const std::string& fragPath) {
    auto mat = std::make_unique<Material>(name);
    if (!mat->loadShader(vertPath, fragPath)) return nullptr;
    Material* ptr = mat.get();
    m_materials.push_back(std::move(mat));
    return ptr;
}

IModel* Engine::AddModel(const std::string& path, const glm::vec3& pos, Material* mat) {
    auto model = std::make_unique<Model>();
    std::string modelPath = path;
    if (!model->LoadObjModel(modelPath)) return nullptr;
    model->SetPosition(pos);
    if (mat) model->SetMaterial(mat);
    IModel* ptr = model.get();
    m_objects.push_back(std::move(model));
    return ptr;
}

void Engine::RemoveModel(int index) {
    if (index < m_objects.size()) {
        m_objects.erase(m_objects.begin() + index);
    }
}

void Engine::RenderAll(glm::mat4& view, glm::mat4& proj) {
    for (auto& obj : m_objects) {
        obj->Render(view, proj);
    }
}

// ========================================
// UI редактора
// ========================================
void Engine::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    RenderSceneEditor();
    RenderMaterialEditor();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Engine::RenderSceneEditor() {
    ImGui::Begin("Scene Editor");

    ImGui::Text("Objects: %zu", m_objects.size());
    ImGui::Separator();

    for (size_t i = 0; i < m_objects.size(); i++) {
        char label[128];
        IModel* model = m_objects[i].get();
        const char* typeName = model ? model->GetTypeName().c_str() : "Unknown";
        glm::vec3 pos = model ? model->GetPosition() : glm::vec3(0);
        
        const char* matName = "No Material";
        if (model) {
            Material* mat = model->GetMaterial();
            if (mat) matName = mat->getName().c_str();
        }
        
        snprintf(label, sizeof(label), "[%zu] %s | Mat: %s | (%.1f, %.1f, %.1f)", 
                 i, typeName, matName, pos.x, pos.y, pos.z);
        
        if (ImGui::Selectable(label, m_selectedObject == (int)i)) {
            m_selectedObject = (int)i;
        }

        RenderRemoveModelPopup(i);
    }

    // Кнопки
    if (ImGui::Button("Add Model")) {
        ImGui::OpenPopup("AddModelPopup");
    }

    RenderCreateModelPopup();

    // Свойства выбранного объекта
    if (m_selectedObject >= 0 && m_selectedObject < (int)m_objects.size()) {
        ImGui::Separator();
        ImGui::Text("Properties:");
        
        IModel* model = m_objects[m_selectedObject].get();
        if (model) {
            // Выбор и изменение материала
            Material* currentMat = model->GetMaterial();
            
            std::vector<std::string> matNamesStr;
            std::vector<const char*> matNamesPtr;
            int currentMatIndex = -1;
            
            matNamesStr.push_back("None");
            for (int mi = 0; mi < m_materials.size(); mi++) {
                matNamesStr.push_back(m_materials[mi]->getName());
                if (currentMat == m_materials[mi].get()) {
                    currentMatIndex = (int)mi + 1;  // +1 из-за "None"
                }
            }
            for (auto& name : matNamesStr) matNamesPtr.push_back(name.c_str());
            
            if (currentMatIndex == -1) currentMatIndex = 0;  // "None"
            
            if (ImGui::Combo("Material", &currentMatIndex, matNamesPtr.data(), (int)matNamesPtr.size())) {
                // Изменили материал
                if (currentMatIndex == 0) {
                    model->SetMaterial(nullptr);
                } else {
                    model->SetMaterial(m_materials[currentMatIndex - 1].get()); // -1 из-за "None"
                }
            }
            
            if (currentMat) {
                ImGui::Text("Shader: %u", currentMat->getShaderProgram());
                ImGui::Text("Textures: %zu", currentMat->textures.size());
            }
            ImGui::Separator();
        }
        
        m_objects[m_selectedObject]->RenderProperties();
    }

    ImGui::End();
}

void Engine::RenderCreateModelPopup() {
    if (ImGui::BeginPopupModal("AddModelPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char pathBuffer[256] = "../resources/models/goblet.obj";

        ImGui::InputText("OBJ Path", pathBuffer, sizeof(pathBuffer));

        ImGui::Text("Material can be set in Scene Editor after creation");

        if (ImGui::Button("Create")) {
            AddModel(pathBuffer, glm::vec3(0, 0, 0), nullptr);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Engine::RenderMaterialEditor() {
    ImGui::Begin("Material Editor");

    ImGui::Text("Materials: %zu", m_materials.size());
    ImGui::Separator();

    for (int i = 0; i < m_materials.size(); i++) {
        char label[128];
        snprintf(label, sizeof(label), "[%d] %s (Shader: %u)", 
                 i, m_materials[i]->getName().c_str(), 
                 m_materials[i]->getShaderProgram());
        
        if (ImGui::Selectable(label, m_selectedMaterial == (int)i)) {
            m_selectedMaterial = (int)i;
        }

        RenderRemoveMaterialPopup(i);
    }

    if (ImGui::Button("Create Material")) {
        ImGui::OpenPopup("CreateMaterialPopup");
    }

    RenderCreateMaterialPopup();

    if (m_selectedMaterial >= 0 && m_selectedMaterial < (int)m_materials.size()) {
        ImGui::Separator();
        
        if (ImGui::Button("Add Texture")) {
            ImGui::OpenPopup("AddTexturePopup");
        }
        
        RenderAddTexturePopup();
        
        ImGui::Separator();
        ImGui::Text("Textures:");
        auto& textures = m_materials[m_selectedMaterial]->textures;
        m_selectedTexture = -1;
        
        for (int t = 0; t < textures.size(); t++) {
            char texLabel[128];
            snprintf(texLabel, sizeof(texLabel), "[%d] %dx%d", t, textures[t]->GetWidth(), textures[t]->GetHeight());
            
            if (ImGui::Selectable(texLabel, m_selectedTexture == (int)t)) {
                m_selectedTexture = (int)t;
            }

            RenderRemoveTexturePopup(t);
            
        }
        
        ImGui::Separator();
        m_materials[m_selectedMaterial]->renderProperties();
    }

    ImGui::End();
}

void Engine::RenderRemoveTexturePopup(int texInd){
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::Button("Remove Texture")) {
            RemoveTextureFromMaterial(m_selectedMaterial, texInd);
            if (m_selectedTexture == texInd) m_selectedTexture = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Engine::RenderRemoveModelPopup(int modelInd) {
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::Button("Remove Model")) {
            RemoveModel(modelInd);
            if (m_selectedObject == modelInd) m_selectedObject = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Engine::RenderRemoveMaterialPopup(int matInd) {
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::Button("Remove Material")) {
            RemoveMaterial(matInd);
            if (m_selectedMaterial == matInd) m_selectedMaterial = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Engine::RenderAddTexturePopup() {
    if (ImGui::BeginPopupModal("AddTexturePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char texPath[256] = "../resources/media/inv.png";

        ImGui::InputText("Texture Path", texPath, sizeof(texPath));

        if (ImGui::Button("Add")) {
            if (m_selectedMaterial >= 0 && m_selectedMaterial < (int)m_materials.size()) {
                m_materials[m_selectedMaterial]->addTexture(texPath);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Engine::RenderCreateMaterialPopup() {
    if (ImGui::BeginPopupModal("CreateMaterialPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char matName[64] = "NewMaterial";
        static char vertPath[256] = "../code/shaders/vertexShaders/first.vert";
        static char fragPath[256] = "../code/shaders/fragmentShaders/first.frag";

        ImGui::InputText("Name", matName, sizeof(matName));
        ImGui::InputText("Vertex Shader", vertPath, sizeof(vertPath));
        ImGui::InputText("Fragment Shader", fragPath, sizeof(fragPath));

        if (ImGui::Button("Create")) {
            Material* mat = CreateMaterial(matName, vertPath, fragPath);
            if (mat) {
                std::cout << "Material created: " << matName << "\n";
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Engine::RemoveMaterial(int index) {
    if (index >= m_materials.size()) return;
    
    for (auto& obj : m_objects) {
        IModel* model = obj.get();
        if (model && model->GetMaterial() == m_materials[index].get()) {
            model->SetMaterial(nullptr);
        }
    }
    
    m_materials.erase(m_materials.begin() + index);
    std::cout << "Material removed\n";
}

void Engine::RemoveTextureFromMaterial(int matIndex, int texIndex) {
    if (matIndex >= m_materials.size()) return;
    auto& textures = m_materials[matIndex]->textures;
    if (texIndex >= textures.size()) return;
    textures.erase(textures.begin() + texIndex);
    std::cout << "Texture removed from material\n";
}

// ========================================
// Очистка
// ========================================
void Engine::Shutdown() {
    m_objects.clear();
    m_materials.clear();
    m_grid.reset();
    m_textRenderer.reset();

    if (m_gridShader) glDeleteProgram(m_gridShader);
    if (m_fpsShader) glDeleteProgram(m_fpsShader);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (m_glContext) SDL_GL_DestroyContext(m_glContext);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}