#include "Mesh.h"
#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"
#include "unordered_map"
#include "unordered_set"


void Mesh::MeshToTriangles(const Mesh& mesh,
    std::vector<glm::vec3>& outPositions,
    std::vector<glm::vec3>& outNormals,
    std::vector<unsigned int>& outIndices,
    std::vector<unsigned int>& outEdgeIndices
)
{
    outPositions.clear();
    outNormals.clear();
    outIndices.clear();
    outEdgeIndices.clear();
    //Unique edge set to avoid duplicates
    std::unordered_set<uint64_t> edgeSet;

    if (!mesh.flatShading)
    {
        std::unordered_map<const Vertex*, unsigned int> vertexToIndex;

        unsigned int index = 0;
        //Push all vertices to the vertex buffer
        for (auto& v : mesh.vertices) {
            outPositions.push_back(v->position);
            outNormals.push_back(v->normal);
            //Add all vertices to a dictionary with an increasing key
            vertexToIndex[v.get()] = index++;
        }


        //Loop through all faces
        for (auto& f : mesh.faces) {
            std::vector<unsigned int> faceIndices;
            const HalfEdge* start = f->edge;
            const HalfEdge* e = start;

            //Loop through each half edge on the face
            do {
                faceIndices.push_back(vertexToIndex.at(e->origin));
                //calculate edge pairs
                unsigned int i0 = vertexToIndex.at(e->origin);
                unsigned int i1 = vertexToIndex.at(e->next->origin);

                // Sort to avoid duplicate reversed edge pairs
                unsigned int a = std::min(i0, i1);
                unsigned int b = std::max(i0, i1);

                // Unique edge hash
                uint64_t key = (uint64_t)a << 32 | b;

                if (edgeSet.insert(key).second) {
                    outEdgeIndices.push_back(a);
                    outEdgeIndices.push_back(b);
                }

                e = e->next;
            } while (e != start);

            //Triangulate the polygon with fanning
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                outIndices.push_back(faceIndices[0]);
                outIndices.push_back(faceIndices[i]);
                outIndices.push_back(faceIndices[i + 1]);
            }
        }
    }
    else
    {
        //Will result in duplicate vertices, which is intended for flat shading since a vertex can only store one normal
        for (auto& f : mesh.faces) {
            const HalfEdge* start = f->edge;
            const HalfEdge* e = start;

            std::vector<glm::vec3> faceVerts;
            //Push back all connected vertices into vertex buffer for each face
            do {
                faceVerts.push_back(e->origin->position);

                e = e->next;
            } while (e != start);

            // Compute face normal once
            if (faceVerts.size() >= 3) {
                glm::vec3 n = glm::normalize(glm::cross(faceVerts[1] - faceVerts[0],
                    faceVerts[2] - faceVerts[0]));

                // Fan triangulate with duplicated vertices
                for (size_t i = 1; i + 1 < faceVerts.size(); ++i) {
                    glm::vec3 p0 = faceVerts[0];
                    glm::vec3 p1 = faceVerts[i];
                    glm::vec3 p2 = faceVerts[i + 1];


                    unsigned int startIndex = outPositions.size();

                    outPositions.push_back(p0);
                    outPositions.push_back(p1);
                    outPositions.push_back(p2);

                    outNormals.push_back(n);
                    outNormals.push_back(n);
                    outNormals.push_back(n);

                    outIndices.push_back(startIndex);
                    outIndices.push_back(startIndex + 1);
                    outIndices.push_back(startIndex + 2);

                    //Get edge indices
                    //TODO: use a hash map of vertex positions to prevent duplicates
                    outEdgeIndices.push_back(startIndex + 1);
                    outEdgeIndices.push_back(startIndex + 2);
                    if (i == faceVerts.size() - 2) {
                        outEdgeIndices.push_back(startIndex);
                        outEdgeIndices.push_back(startIndex + 2);
                    }
                    if (i == 1) {
                        outEdgeIndices.push_back(startIndex);
                        outEdgeIndices.push_back(startIndex + 1);
                    }
                }
            }
        }
    }
}

void Mesh::ComputeNormals(Mesh& mesh)
{
    // Reset all vertex normals
    for (auto& v : mesh.vertices)
        v->normal = glm::vec3(0.0f);

    // Compute face normals and accumulate into vertex normals
    for (auto& f : mesh.faces)
    {
        HalfEdge* e0 = f->edge;
        if (!e0 || !e0->next || !e0->next->next)
            continue;

        glm::vec3 p0 = e0->origin->position;
        glm::vec3 p1 = e0->next->origin->position;
        glm::vec3 p2 = e0->next->next->origin->position;

        glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

        // Assign to all vertices in this face
        HalfEdge* e = e0;
        do {
            e->origin->normal += normal;
            e = e->next;
        } while (e != e0);
    }

    // Normalize accumulated vertex normals
    for (auto& v : mesh.vertices)
        v->normal = glm::normalize(v->normal);
}

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

void Mesh::OriginToGeometry() {
    int numVertices = vertices.size();
    glm::vec3 cumulativePosition = glm::vec3(0.0f);
    for (auto& v : vertices) {
        cumulativePosition += v->position;
    }
    LocalOrigin = cumulativePosition / (float)numVertices;
}

void Mesh::ClearGPU() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &eboEdges);
}

glm::vec3 Mesh::GetGlobalOrigin() {

    return glm::vec3(Model * glm::vec4(LocalOrigin, 1.0f));
}