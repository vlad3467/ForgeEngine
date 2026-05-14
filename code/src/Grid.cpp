#include "Grid.h"

Grid::Grid(int size, float step) : size(size), step(step) {
    GenerateGrid();
}

Grid::~Grid() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}
    
void Grid::RenderProperties() {
    //Пока хз, что бы добавить, даже не думаю теперь, что нужно
}

void Grid::GenerateGrid() {
    std::vector<float> vertices;
    float halfSize = (size * step) / 2.0f;

    // Генерируем линии вдоль X (параллельные Z)
    for (int i = 0; i <= size; i++) {
        float pos = -halfSize + i * step;
        
        // Горизонтальная линия
        vertices.push_back(-halfSize);  // x1
        vertices.push_back(0.0f);       // y1
        vertices.push_back(pos);        // z1
        
        vertices.push_back(halfSize);   // x2
        vertices.push_back(0.0f);       // y2
        vertices.push_back(pos);        // z2
        
        // Вертикальная линия (параллельная X)
        vertices.push_back(pos);        // x1
        vertices.push_back(0.0f);       // y1
        vertices.push_back(-halfSize);  // z1
        
        vertices.push_back(pos);        // x2
        vertices.push_back(0.0f);       // y2
        vertices.push_back(halfSize);   // z2
    }

    vertexCount = vertices.size() / 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void Grid::Render(glm::mat4& view, glm::mat4& projection) {

    GLuint shaderProgram = material->getShaderProgram();

    if (!shaderProgram || !VAO) return;

    glUseProgram(shaderProgram);
    
    glm::mat4 model = glm::mat4(1.0f);
    
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uniModel"), 
                       1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uniView"), 
                       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uniProjection"), 
                       1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
    glUseProgram(0);
}