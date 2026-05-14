#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <map>

class ObjParser {
private:
    std::vector<glm::fvec3> temp_positions;
    std::vector<glm::fvec2> temp_texcoords;
    std::vector<glm::fvec3> temp_normals;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    struct VertexKey {
        int pos, tex, norm;
        bool operator<(const VertexKey& other) const {
            if (pos != other.pos) return pos < other.pos;
            if (tex != other.tex) return tex < other.tex;
            return norm < other.norm;
        }
    };
    std::map<VertexKey, unsigned int> vertexCache;
    
    std::string currentMaterial;
    
public:
    bool LoadObjModel(const std::string& filename);
    const std::vector<float>& GetVertices() const { return vertices; }
    const std::vector<unsigned int>& GetIndices() const { return indices; }

    
private:
    void ParseVertex(const std::string& line);
    void ParseTexCoord(const std::string& line);
    void ParseNormal(const std::string& line);
    void ParseFace(const std::string& line);
    unsigned int GetOrCreateVertex(int posIdx, int texIdx, int normIdx);
};