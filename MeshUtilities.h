#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Face.h"
class Mesh;

void MeshToTriangles(const Mesh& mesh,
    std::vector<glm::vec3>& outPositions,
    std::vector<glm::vec3>& outNormals,
    std::vector<unsigned int>& outIndices,
    std::vector<unsigned int>& outEdgeIndices);

void ComputeNormals(Mesh& mesh);

bool RayTriangle(const glm::vec3& orig, const glm::vec3& dir,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
    float& tOut);

bool PickMesh(Mesh& mesh,
    glm::vec3 rayOrigin,
    glm::vec3 rayDir,
    float& outDist,
    Face*& outFace);