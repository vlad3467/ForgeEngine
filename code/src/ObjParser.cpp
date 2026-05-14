#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

#include "ObjParser.h"

bool ObjParser::LoadObjModel(const std::string& filename) {

    temp_positions.clear();
    temp_texcoords.clear();
    temp_normals.clear();
    vertices.clear();
    indices.clear();
    vertexCache.clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    
    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string prefix;

        iss >> prefix;
        
        if (prefix == "v") {
            ParseVertex(line);
        } else if (prefix == "vt") {
            ParseTexCoord(line);
        } else if (prefix == "vn") {
            ParseNormal(line);
        } else if (prefix == "f") {
            ParseFace(line);
        } else if (prefix == "mtllib") {
            // Загрузка материалов — пока игнор
        } else if (prefix == "usemtl") {
            iss >> currentMaterial;
        } else if (prefix == "o" || prefix == "g") {
            // Имя объекта или группы — пока игнор
        } else if (prefix == "s") {
            // Smooth shading — пока игнор
        }
    }
    
    std::cout << "Loaded OBJ file: " << filename << std::endl;
    std::cout << "  Vertices: " << temp_positions.size() << std::endl;
    std::cout << "  TexCoords: " << temp_texcoords.size() << std::endl;
    std::cout << "  Normals: " << temp_normals.size() << std::endl;

    temp_positions.clear();
    temp_texcoords.clear();
    temp_normals.clear();
    vertexCache.clear();
    
    return true;
}

void ObjParser::ParseVertex(const std::string& line) {
    std::istringstream iss(line);
    std::string v;
    glm::vec3 vertex;
    
    if (iss >> v >> vertex.x >> vertex.y >> vertex.z) {
        temp_positions.push_back(vertex);
    } 
    else {
        std::cerr << "Warning: Failed to parse vertex: " << line << std::endl;
    }
}

void ObjParser::ParseTexCoord(const std::string& line) {
    std::istringstream iss(line);
    std::string vt;
    glm::vec2 texcoord;
    
    if (iss >> vt >> texcoord.x >> texcoord.y) {
        temp_texcoords.push_back(texcoord);
    } 
    else {
        std::cerr << "Warning: Failed to parse texcoord: " << line << std::endl;
    }
}

void ObjParser::ParseNormal(const std::string& line) {
    std::istringstream iss(line);
    std::string vn;
    glm::vec3 normal;
    
    if (iss >> vn >> normal.x >> normal.y >> normal.z) {
        temp_normals.push_back(normal);
    } 
    else {
        std::cerr << "Warning: Failed to parse normal: " << line << std::endl;
    }
}

unsigned int ObjParser::GetOrCreateVertex(int posIdx, int texIdx, int normIdx) {
    VertexKey key{posIdx, texIdx, normIdx};
    
    auto it = vertexCache.find(key);
    if (it != vertexCache.end()) {
        return it->second;
    }
    
    unsigned int newIndex = static_cast<unsigned int>(vertices.size() / 8);
    vertexCache[key] = newIndex;
    
    const glm::vec3& pos = temp_positions[posIdx];
    vertices.push_back(pos.x);
    vertices.push_back(pos.y);
    vertices.push_back(pos.z);

    const glm::vec2& tex = temp_texcoords[texIdx];
    vertices.push_back(tex.x);
    vertices.push_back(tex.y);
    
    const glm::vec3& norm = temp_normals[normIdx];
    vertices.push_back(norm.x);
    vertices.push_back(norm.y);
    vertices.push_back(norm.z);
    
    return newIndex;
}

void ObjParser::ParseFace(const std::string& line) {
    std::istringstream iss(line);
    std::string f;
    iss >> f;
    
    std::vector<glm::ivec3> faceVertices;
    std::string vertexData;

    while (iss >> vertexData) {
        glm::ivec3 faceIndices = {-1, -1, -1};
        
        for (char& c : vertexData) {
            if (c == '/') c = ' ';
        }
        
        std::istringstream viss(vertexData);
        
        if (viss >> faceIndices.x) {
            faceIndices.x--;
            if (viss >> faceIndices.y) {
                faceIndices.y--;
                if (viss >> faceIndices.z) {
                    faceIndices.z--;
                }
            }
        }
        
        faceVertices.push_back(faceIndices);
    }
    
    if (faceVertices.size() != 3) {
        std::cerr << "Warning: Face is not a triangle (" << faceVertices.size() 
                  << " vertices): " << line << std::endl;
        return;
    }
    
    for (int i = 0; i < 3; i++) {
        int posIdx = faceVertices[i].x;
        int texIdx = faceVertices[i].y;
        int normIdx = faceVertices[i].z;
        
        unsigned int vertexIndex = GetOrCreateVertex(posIdx, texIdx, normIdx);
        indices.push_back(vertexIndex);
    }
}