#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <map>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

int gWindowWidth = 1080;
int gWindowHeight = 720;

SDL_Window* gWindow = nullptr;
SDL_GLContext gGLcontext;

bool init(){
    bool success = true;
    if (SDL_Init( SDL_INIT_VIDEO) == false){
        SDL_Log("Unable to init SDL, SDL error: %s\n", SDL_GetError());
        success = false;
    }
    else{
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
        if (gWindow = SDL_CreateWindow("THE BEGINNING", gWindowWidth, gWindowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE); gWindow == nullptr){
            SDL_Log("Unable to init gWindow, SDL error: %s\n", SDL_GetError());
            success = false;
        }
        else{
            gGLcontext = SDL_GL_CreateContext(gWindow);
            if (!gGLcontext){
                SDL_Log("Unable to create OpenGL context, SDL error: %s\n", SDL_GetError());
                success = false;
            }
            else{
                if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)){
                    SDL_Log("Unable to load OpenGL functions, SDL error: %s\n", SDL_GetError());
                    success = false;
                }
                else {
                    IMGUI_CHECKVERSION();
                    ImGui::CreateContext();
                    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
                    ImGui::GetStyle().ScaleAllSizes(main_scale);
                    
                    ImGui::StyleColorsDark();
                    
                    if (!ImGui_ImplSDL3_InitForOpenGL(gWindow, gGLcontext)) {
                        SDL_Log("Failed to initialize ImGui SDL3 backend");
                        success = false;
                    }
                    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
                        SDL_Log("Failed to initialize ImGui OpenGL3 backend");
                        success = false;
                    }
                    
                    SDL_Log("ImGui successfully initialized!");
                }
            }
        }
    }
    return success;   
}

void close(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    
    if (gGLcontext) {
        SDL_GL_DestroyContext(gGLcontext);
    }
    if (gWindow) {
        SDL_DestroyWindow(gWindow);
    }
    SDL_Quit();
}

void shaderCompileStatus(GLuint shader){
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

GLuint compileShaderProgram(const char* vertexShaderSource, const char* fragShaderSource){
    size_t vertexFileSize;
    char* vertexShaderFile = (char*)SDL_LoadFile(vertexShaderSource, &vertexFileSize);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderFile, NULL);
    glCompileShader(vertexShader);
    shaderCompileStatus(vertexShader);
            
    size_t fragFileSize;
    char* fragShaderFile = (char*)SDL_LoadFile(fragShaderSource, &fragFileSize);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderFile, NULL);
    glCompileShader(fragShader);
    shaderCompileStatus(fragShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    SDL_free(vertexShaderFile);
    SDL_free(fragShaderFile);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    return shaderProgram;
}

GLuint createTexture(SDL_Surface* surface){

    GLenum formatSurface = GL_RGBA;
    if (surface->format == SDL_PIXELFORMAT_RGB24) {
        formatSurface = GL_RGB;
    }

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, formatSurface, surface->w, surface->h, 0, formatSurface, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void updateTexture(GLuint texture, SDL_Surface* surface) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint VAO, VBO;

struct Character {
    unsigned int textureID;
    glm::ivec2   size;
    glm::ivec2   bearing;
    long advance;
};

class Shader{
};

std::map<char, Character> Characters;

void RenderText(GLuint shaderProgram, GLuint VBO, GLuint VAO, std::string text, float x, float y, float scale, glm::vec3 color)
{
    glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
    glUniform1i(glGetUniformLocation(shaderProgram, "textTexture"), 0);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int argc, char* argv[]) {

    if (init() == false){
        SDL_Log("Unable to init program, SDL error: %s\n", SDL_GetError());
        return -1;
    }
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "../resources/fonts/lazy.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        return -1;
    }

    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;  
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  -5.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    bool quit = false;

    glad_glViewport(0, 0, gWindowWidth, gWindowHeight);

    SDL_Event e;
    SDL_zero(e);

    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    std::string pathImage1 = "../resources/media/invincible.png";
    SDL_Surface* surfaceImage1 = IMG_Load(pathImage1.c_str());
    if (!surfaceImage1) {
    SDL_Log("Failed to load image...");
    close();
    return -1;
    }
    std::string pathImage2 = "../resources/media/green.png";
    SDL_Surface* surfaceImage2 = IMG_Load(pathImage2.c_str());
    if (!surfaceImage2) {
    SDL_Log("Failed to load image...");
    close();
    return -1;
    }

    GLuint texture1 = createTexture(surfaceImage1);
    GLuint texture2 = createTexture(surfaceImage2);

    const char* cubeVertexShaderSource = "../code/shaders/vertexShaders/first.vert";
    const char* cubeFragShaderSource = "../code/shaders/fragmentShaders/first.frag";

    GLuint shaderProgram;
    shaderProgram = compileShaderProgram(cubeVertexShaderSource, cubeFragShaderSource);

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    float cameraSpeed = 0.15;

    float startTime = SDL_GetTicks();

    float fps = 0;
    std::string fpsText = "FPS - 0";

    const char* fpsVertexShaderSource = "../code/shaders/vertexShaders/fpsText.vert";
    const char* fpsFragShaderSource = "../code/shaders/fragmentShaders/fpsText.frag";
    GLuint fpsShaderProgram = compileShaderProgram(fpsVertexShaderSource, fpsFragShaderSource);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    GLuint fpsVBO, fpsVAO;
    glGenBuffers(1, &fpsVBO);
    glGenVertexArrays(1, &fpsVAO);

    glBindVertexArray(fpsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fpsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glUseProgram(fpsShaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)gWindowWidth, (float)gWindowHeight, 0.0f);
    GLint projLoc = glGetUniformLocation(fpsShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(0);

    while(quit == false){

        GLfloat deltaTime = SDL_GetTicks();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        fps++;

        if (((deltaTime/1000.0) - (startTime/1000.0)) > 0.3){
            fpsText = "FPS - " + std::to_string(fps/0.3);
            startTime = deltaTime;
            fps = 0;
        }

        while(SDL_PollEvent(&e) == true){
            ImGui_ImplSDL3_ProcessEvent(&e);
            
            if (e.type == SDL_EVENT_QUIT){
                quit = true;
            }
            
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            
            if (!io.WantCaptureKeyboard) {
                switch (e.key.key){
                    case SDLK_W: cameraPos += cameraFront * cameraSpeed; break;
                    case SDLK_S: cameraPos -= cameraFront * cameraSpeed; break;
                    case SDLK_A: cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; break;
                    case SDLK_D: cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; break;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        glEnable(GL_DEPTH_TEST);

        glUseProgram(shaderProgram);

        glm::mat4 view{1.0};

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::mat4 perspective{1.0}; 
        perspective = glm::perspective(glm::radians(45.0f), ((float)gWindowWidth/gWindowHeight), 0.1f, 100.0f);

        GLuint modelLocation = glGetUniformLocation(shaderProgram, "uniModel");
        GLuint viewLocation = glGetUniformLocation(shaderProgram, "uniView");
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        GLuint projectionLocation = glGetUniformLocation(shaderProgram, "uniPerspective");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspective));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture1"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture2"), 1);

        glBindVertexArray(VAO);
        for(GLuint i = 0; i < 10; i++)
        {
            glm::mat4 model{1.0};
            model = glm::translate(model, cubePositions[i]);
            GLfloat angle = deltaTime/500 * i; 
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glDisable(GL_DEPTH_TEST);
        RenderText(fpsShaderProgram, fpsVBO, fpsVAO, fpsText, 0, 0, 1, glm::vec3(0.5, 0.8f, 0.2f));

        ImGui::Begin("a Very Temporary User Interface");
        ImGui::Text("Hello from ImGui!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(gWindow);
    }

    close();
    return 0;
}