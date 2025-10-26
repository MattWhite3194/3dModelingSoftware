#include "MeshUtilities.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Face.h"
#include "HalfEdge.h"
#include <unordered_set>

void MeshToTriangles(const Mesh& mesh,
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


/*To calculate orientation for a messed up mesh :
Select one face arbitrarily, consider that it winds correctly, correct evey other face to correct it's orientation.
Compute the meshes origin, the whole mesh or selected faces, flip all faces once to point inside towards that origin.
*/
void ComputeNormals(Mesh& mesh)
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

/// <summary>
/// Determines if a ray intersects a triangle (Moller Trumbore)
/// </summary>
/// <param name="orig"></param>
/// <param name="dir"></param>
/// <param name="v0"></param>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <param name="tOut"></param>
/// <returns></returns>
bool RayTriangle(const glm::vec3& orig, const glm::vec3& dir,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
    float& tOut)
{
    const float EPSILON = 0.000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec3 pvec = glm::cross(dir, edge2);
    float det = glm::dot(edge1, pvec);
    if (fabs(det) < EPSILON) return false;

    float invDet = 1.0f / det;
    glm::vec3 tvec = orig - v0;
    float u = glm::dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return false;

    glm::vec3 qvec = glm::cross(tvec, edge1);
    float v = glm::dot(dir, qvec) * invDet;
    if (v < 0 || u + v > 1) return false;

    tOut = glm::dot(edge2, qvec) * invDet;
    return (tOut > EPSILON);
}

/// <summary>
/// Determines if a ray intersects a mesh, outputs the face it intersected, and the distance from the ray
/// </summary>
/// <param name="mesh"></param>
/// <param name="rayOrigin"></param>
/// <param name="rayDir"></param>
/// <param name="outDist"></param>
/// <param name="outFace"></param>
/// <returns></returns>
bool PickMesh(Mesh& mesh, glm::vec3 rayOrigin, glm::vec3 rayDir, float& outDist, Face*& outFace) {
    glm::mat4 model = mesh.GetModelMatrix();
    glm::mat4 invModel = glm::inverse(model);

    // Transform ray origin and direction into mesh-local space
    glm::vec3 localOrig = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));

    outFace = nullptr;
    outDist = FLT_MAX;

    for (auto& f : mesh.faces)
    {
        // Fan triangulate through all edges
        const HalfEdge* start = f->edge;
        const HalfEdge* e = start;
        const HalfEdge* e1 = e->next;
        const HalfEdge* e2 = e1->next;

        while (e2 != start) {
            // Test triangle (e, e1, e2)
            glm::vec3 v0 = e->origin->position;
            glm::vec3 v1 = e1->origin->position;
            glm::vec3 v2 = e2->origin->position;

            float t;
            if (RayTriangle(localOrig, localDir, v0, v1, v2, t) && t < outDist) {
                glm::vec3 hitLocal = localOrig + localDir * t;
                glm::vec3 hitWorld = glm::vec3(model * glm::vec4(hitLocal, 1.0f));
                float tWorld = glm::length(hitWorld - rayOrigin);
                outDist = tWorld;
                outFace = f.get();
            }

            // Move forward in fan
            e1 = e2;
            e2 = e2->next;
        }
    }

    return (outFace != nullptr);
}