#include <Model.h>
#include <iostream>

void Model::Render(glm::mat4& view, glm::mat4& proj){

    if (!VAO) return;
    if (!material) return;

    GLuint shaderProgram = material->getShaderProgram();

    glUseProgram(shaderProgram);

    glm::mat4 model{1.0};
    model = glm::translate(model, position);
    model = glm::rotate(model, (float)glm::radians(rotation.w), glm::vec3(rotation.x,rotation.y,rotation.z));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uniModel"), 
                       1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uniView"), 
                       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uniProjection"), 
                       1, GL_FALSE, glm::value_ptr(proj));

    material->bindTextures();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

bool Model::LoadObjModel(std::string& pathImage){

    // Если модель уже загружена — удаляем старые буферы
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
    }

    ObjParser objParser;
    if (!objParser.LoadObjModel(pathImage)){
        std::cout << "ERROR::MODEL: Could not load model" << pathImage << std::endl;
        return false;
    }
    vertices = objParser.GetVertices();
    indices = objParser.GetIndices();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW); 
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(5*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void Model::RenderProperties() {
        ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f);
        ImGui::DragFloat4("Rotation", glm::value_ptr(rotation), 0.1f);
}

Model::~Model() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}