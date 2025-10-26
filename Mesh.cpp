#include "Mesh.h"
#include "MeshUtilities.h"
#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"
#include "unordered_map"

Vertex* Mesh::addVertex(const glm::vec3& pos) {
    vertices.push_back(std::make_unique<Vertex>());
    Vertex* v = vertices.back().get();
    v->position = pos;
    return v;
}

Face* Mesh::addFace(const std::vector<Vertex*>& verts) {
    if (verts.size() < 3) return nullptr;

    faces.push_back(std::make_unique<Face>());
    Face* face = faces.back().get();

    std::vector<HalfEdge*> edges;
    edges.reserve(verts.size());

    // Create one edge per vertex
    for (size_t i = 0; i < verts.size(); ++i) {
        halfEdges.push_back(std::make_unique<HalfEdge>());
        HalfEdge* e = halfEdges.back().get();

        e->origin = verts[i];
        e->face = face;
        edges.push_back(e);

        if (!verts[i]->outgoing)
            verts[i]->outgoing = e;
    }

    // Link edges circularly
    for (size_t i = 0; i < edges.size(); ++i) {
        edges[i]->next = edges[(i + 1) % edges.size()];
    }

    face->edge = edges[0];

    // Twin linking (works the same as before)
    for (auto* e : edges) {
        auto key = std::make_pair(e->next->origin, e->origin);
        auto reverseKey = std::make_pair(e->origin, e->next->origin);

        if (edgeMap.count(reverseKey)) {
            e->twin = edgeMap[reverseKey];
            e->twin->twin = e;
        }
        else {
            edgeMap[key] = e;
        }
    }

    return face;
}

void Mesh::RebuildRenderData() {
    renderPositions.clear();
    renderIndices.clear();
    MeshToTriangles(*this, renderPositions, renderNormals, renderIndices, edgeIndices);
    gpuDirty = true;
}

void Mesh::UploadToGPU()
{
    if (!gpuDirty) return;

    // Compute normals first
    ComputeNormals(*this);
    MeshToTriangles(*this, renderPositions, renderNormals, renderIndices, edgeIndices);

    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);
    if (!ebo) glGenBuffers(1, &ebo);
    if (!eboEdges) glGenBuffers(1, &eboEdges);

    struct VertexData {
        glm::vec3 pos;
        glm::vec3 normal;
    };

    std::vector<VertexData> vertexData(renderPositions.size());
    for (size_t i = 0; i < renderPositions.size(); ++i) {
        vertexData[i].pos = renderPositions[i];
        vertexData[i].normal = renderNormals[i];
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(VertexData),
        vertexData.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        renderIndices.size() * sizeof(unsigned int),
        renderIndices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboEdges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        edgeIndices.size() * sizeof(unsigned int),
        edgeIndices.data(),
        GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    gpuDirty = false;
}

void Mesh::Draw(Shader& shader) {
    if (gpuDirty)
        RebuildRenderData(), UploadToGPU();
    if (transformDirty)
        UpdateModelMatrix();

    shader.use();
    shader.setBool("lightingEnabled", true);
    shader.setMat4("model", GetModelMatrix());
    shader.setVec4("objectColor", ObjectColor);

    //Draw Faces
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, renderIndices.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::DrawEdges(Shader& shader) {
    if (gpuDirty)
        RebuildRenderData(), UploadToGPU();
    if (transformDirty)
        UpdateModelMatrix();

    shader.use();
    shader.setMat4("model", GetModelMatrix());
    shader.setVec4("edgeColor", selected ? glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    shader.setFloat("lineWidth", 2.0f);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);  // Pull edges toward the camera
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboEdges);
    glDrawElements(GL_LINES, edgeIndices.size(), GL_UNSIGNED_INT, 0);
    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}