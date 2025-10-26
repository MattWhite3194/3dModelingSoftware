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
#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"

/// <summary>
/// Used for hashing a pair of vertices when inserting a half edge into a lookup table
/// </summary>
struct PairHash {
    size_t operator()(const std::pair<Vertex*, Vertex*>& p) const noexcept {
        return std::hash<Vertex*>()(p.first) ^ (std::hash<Vertex*>()(p.second) << 1);
    }
};

class Mesh {
public:
    //TODO: Object only has one stored vec3 per transformation
    //transformations along axis are affected by objects rotation, only affects the single model matrix.
    //Mesh Data
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<HalfEdge>> halfEdges;
    std::vector<std::unique_ptr<Face>> faces;
    std::string name;

    //Transformations
    glm::vec3 Scale = glm::vec3(1.0f);
    glm::vec3 Rotation = glm::vec3(0.0f);
    glm::vec3 Translation = glm::vec3(0.0f);

    //Drawing
    GLuint vao = 0, vbo = 0, ebo = 0, eboEdges = 0;
    bool gpuDirty = true; // needs to re-upload?
    bool transformDirty = false;
    std::vector<glm::vec3> renderPositions;
    std::vector<unsigned int> renderIndices;
    std::vector<glm::vec3> renderNormals;
    std::vector<unsigned int> edgeIndices;
    /// <summary>
    /// Used to store an edge as a pair of vertices for quick lookup when finding the adjacent halfedge (twin)
    /// </summary>
    std::unordered_map<std::pair<Vertex*, Vertex*>, HalfEdge*, PairHash> edgeMap;
    glm::vec4 ObjectColor = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    bool flatShading = true;
    bool selected = false;

    glm::mat4 GetModelMatrix() {
        return Model;
    }
    Vertex* addVertex(const glm::vec3& pos);
    Face* addFace(const std::vector<Vertex*>& verts);
    void RebuildRenderData();
    void UploadToGPU();
    void Draw(Shader& shader);
    void ScaleBy(glm::vec3 factor) {
        Scale *= factor;
        UpdateModelMatrix();
    }
    void Rotate(glm::vec3 rotation) {
        Rotation += rotation;
        UpdateModelMatrix();
    }
    void Translate(glm::vec3 translation) {
        Translation += translation;
        UpdateModelMatrix();
    }
private:
    //Transformations
    glm::mat4 Model = glm::mat4(1.0f);
    void UpdateModelMatrix() {
        glm::mat4 model(1.0f);

        model = glm::translate(model, Translation);

        model = glm::rotate(model, glm::radians<float>(Rotation.z), glm::vec3(0, 0, 1));
        model = glm::rotate(model, glm::radians<float>(Rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians<float>(Rotation.x), glm::vec3(1, 0, 0));

        model = glm::scale(model, Scale);

        Model = model;
    }
};
#endif