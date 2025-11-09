#pragma once

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

    Mesh Clone() const
    {
        Mesh copy;
        copy.name = name;
        copy.Scale = Scale;
        copy.Rotation = Rotation;
        copy.Translation = Translation;
        copy.ObjectColor = ObjectColor;
        copy.flatShading = flatShading;
        copy.selected = selected;
        copy.renderPositions = renderPositions;
        copy.renderIndices = renderIndices;
        copy.renderNormals = renderNormals;
        copy.edgeIndices = edgeIndices;
        copy.gpuDirty = true; // force rebuild on GPU
        copy.Model = Model;

        // --- Step 1: Duplicate all objects ---
        std::unordered_map<const Vertex*, Vertex*> vertexMap;
        std::unordered_map<const HalfEdge*, HalfEdge*> halfEdgeMap;
        std::unordered_map<const Face*, Face*> faceMap;

        // Duplicate vertices
        for (const auto& v : vertices) {
            copy.vertices.push_back(std::make_unique<Vertex>(*v));
            vertexMap[v.get()] = copy.vertices.back().get();
        }

        // Duplicate faces
        for (const auto& f : faces) {
            copy.faces.push_back(std::make_unique<Face>(*f));
            faceMap[f.get()] = copy.faces.back().get();
        }

        // Duplicate half-edges
        for (const auto& he : halfEdges) {
            copy.halfEdges.push_back(std::make_unique<HalfEdge>(*he));
            halfEdgeMap[he.get()] = copy.halfEdges.back().get();
        }

        // --- Step 2: Fix up internal pointers ---
        for (const auto& he : halfEdges) {
            HalfEdge* newHe = halfEdgeMap[he.get()];

            if (he->next) newHe->next = halfEdgeMap[he->next];
            if (he->twin) newHe->twin = halfEdgeMap[he->twin];
            if (he->origin) newHe->origin = vertexMap[he->origin];
            if (he->face) newHe->face = faceMap[he->face];
        }

        for (const auto& f : faces) {
            Face* newF = faceMap[f.get()];
            if (f->edge) newF->edge = halfEdgeMap[f->edge];
        }

        for (const auto& v : vertices) {
            Vertex* newV = vertexMap[v.get()];
            if (v->outgoing) newV->outgoing = halfEdgeMap[v->outgoing];
        }

        // --- Step 3: Rebuild edge map ---
        for (const auto& [edgePair, he] : edgeMap) {
            Vertex* newV1 = vertexMap[edgePair.first];
            Vertex* newV2 = vertexMap[edgePair.second];
            HalfEdge* newHe = halfEdgeMap[he];
            copy.edgeMap[{newV1, newV2}] = newHe;
        }

        return copy;
    }

    glm::mat4 GetModelMatrix() {
        return Model;
    }

    static void MeshToTriangles(const Mesh& mesh,
        std::vector<glm::vec3>& outPositions,
        std::vector<glm::vec3>& outNormals,
        std::vector<unsigned int>& outIndices,
        std::vector<unsigned int>& outEdgeIndices);

    static void ComputeNormals(Mesh& mesh);

    Vertex* addVertex(const glm::vec3& pos);

    Face* addFace(const std::vector<Vertex*>& verts);

    void RebuildRenderData();

    void UploadToGPU();

    void Draw(Shader& shader);

    void DrawEdges(Shader& shader);

    void OriginToGeometry();

    glm::vec3 GetGlobalOrigin();

    Mesh() = default;

    ~Mesh() {
        ClearGPU();
    }

    //disable copying (Mesh a = b)
    Mesh(const Mesh&) = delete;
    //delete the copy assignment operator
    Mesh& operator=(const Mesh&) = delete;

    //move constructor
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

private:
    //Transformations
    glm::mat4 Model = glm::mat4(1.0f);
    //LocalOrigin
    glm::vec3 LocalOrigin;

    void ClearGPU();

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