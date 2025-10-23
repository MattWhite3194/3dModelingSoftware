#include "MeshUtilities.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Face.h"
#include "HalfEdge.h"

void MeshToTriangles(const Mesh& mesh,
    std::vector<glm::vec3>& outPositions,
    std::vector<glm::vec3>& outNormals,
    std::vector<unsigned int>& outIndices)
{
    outPositions.clear();
    outNormals.clear();
    outIndices.clear();

    if (!mesh.flatShading)
    {
        std::unordered_map<const Vertex*, unsigned int> vertexToIndex;

        unsigned int index = 0;
        for (auto v : mesh.vertices) {
            outPositions.push_back(v->position);
            outNormals.push_back(v->normal);
            vertexToIndex[v] = index++;
        }

        for (auto f : mesh.faces) {
            std::vector<unsigned int> faceIndices;
            const HalfEdge* start = f->edge;
            const HalfEdge* e = start;

            do {
                faceIndices.push_back(vertexToIndex.at(e->origin));
                e = e->next;
            } while (e != start);

            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                outIndices.push_back(faceIndices[0]);
                outIndices.push_back(faceIndices[i]);
                outIndices.push_back(faceIndices[i + 1]);
            }
        }
    }
    else
    {
        for (auto f : mesh.faces) {
            const HalfEdge* start = f->edge;
            const HalfEdge* e = start;

            std::vector<glm::vec3> faceVerts;
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
                }
            }
        }
    }
}

void ComputeNormals(Mesh& mesh)
{
    // Reset all vertex normals
    for (auto v : mesh.vertices)
        v->normal = glm::vec3(0.0f);

    // Compute face normals and accumulate into vertex normals
    for (auto f : mesh.faces)
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
    for (auto v : mesh.vertices)
        v->normal = glm::normalize(v->normal);
}