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
    glm::vec3 GetScale() {
        return Scale;
    }
    glm::vec3 GetTranslation() {
        return Translation;
    }
    glm::vec3 GetRotation() {
        return Rotation;
    }
private:
    //Transformations
    glm::vec3 Scale = glm::vec3(1.0f);
    glm::vec3 Rotation = glm::vec3(0.0f);
    glm::vec3 Translation = glm::vec3(1.0f);
    glm::mat4 Model = glm::mat4(1.0f);
    void UpdateModelMatrix() {
        glm::mat4 model(1.0f);

        // Scale
        model = glm::scale(model, Scale);

        // Rotation (Z * Y * X is standard)
        model = glm::rotate(model, Rotation.z, glm::vec3(0, 0, 1));
        model = glm::rotate(model, Rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, Rotation.x, glm::vec3(1, 0, 0));

        // Translation (last step!)
        model = glm::translate(model, Translation);

        Model = model;
    }
};
#endif