#include "Mesh.h"
#include "MeshUtilities.h"
#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"

Vertex* Mesh::addVertex(const glm::vec3& pos) {
    Vertex* v = new Vertex();
    v->position = pos;
    vertices.push_back(v);
    return v;
}

Face* Mesh::addFace(const std::vector<Vertex*>& verts) {
    if (verts.size() < 3) return nullptr;

    Face* face = new Face();
    faces.push_back(face);

    std::vector<HalfEdge*> edges;
    edges.reserve(verts.size());

    // Create one edge per vertex
    for (size_t i = 0; i < verts.size(); ++i) {
        HalfEdge* e = new HalfEdge();
        e->origin = verts[i];
        e->face = face;
        halfEdges.push_back(e);
        edges.push_back(e);

        if (!verts[i]->outgoing)
            verts[i]->outgoing = e;
    }

    // Link edges circularly
    for (size_t i = 0; i < edges.size(); ++i) {
        edges[i]->next = edges[(i + 1) % edges.size()];
    }

    face->edge = edges[0];

    // Build twin links (simple lookup by edge pair)
    static std::unordered_map<uint64_t, HalfEdge*> edgeMap;

    auto key = [](Vertex* a, Vertex* b) -> uint64_t {
        return (reinterpret_cast<uint64_t>(a) << 32) ^
            reinterpret_cast<uint64_t>(b);
        };

    for (auto* e : edges) {
        uint64_t k = key(e->next->origin, e->origin); // reversed direction
        if (edgeMap.count(k)) {
            e->twin = edgeMap[k];
            e->twin->twin = e;
        }
        else {
            edgeMap[key(e->origin, e->next->origin)] = e;
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

    glUseProgram(shader.ID);
    shader.setMat4("model", GetModelMatrix());
    shader.setVec4("objectColor", ObjectColor);

    //Draw Faces
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, renderIndices.size(), GL_UNSIGNED_INT, 0);

    //TODO: Draw Edges
    glLineWidth(1.5f);
    shader.setVec4("objectColor", selected ? glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboEdges);
    glDrawElements(GL_LINES, edgeIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}