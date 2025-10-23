#ifndef MESH_H
#define MESH_H
#include <glad/glad.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_s.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Vertex;
struct HalfEdge;
struct Face;

class Mesh {
public:
    //Mesh Data
    std::vector<Vertex*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Face*> faces;
    
    //Drawing
    GLuint vao = 0, vbo = 0, ebo = 0, eboEdges = 0;
    bool gpuDirty = true; // needs to re-upload?
    std::vector<glm::vec3> renderPositions;
    std::vector<unsigned int> renderIndices;
    std::vector<glm::vec3> renderNormals;
    std::vector<unsigned int> edgeIndices;
    bool flatShading = true;

    //Translations
    glm::mat4 Scale = glm::mat4(1.0f);
    glm::mat4 Rotation = glm::mat4(1.0f);
    glm::mat4 Translation = glm::mat4(1.0f);
    glm::vec4 ObjectColor = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);

    glm::mat4 GetModelMatrix() {
        return Scale * Rotation * Translation;
    }
    Vertex* addVertex(const glm::vec3& pos);
    Face* addFace(const std::vector<Vertex*>& verts);
    void RebuildRenderData();
    void UploadToGPU();
    void Draw(Shader& shader);
};
#endif