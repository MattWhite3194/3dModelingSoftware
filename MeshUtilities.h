#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Mesh;

void MeshToTriangles(const Mesh& mesh,
    std::vector<glm::vec3>& outPositions,
    std::vector<glm::vec3>& outNormals,
    std::vector<unsigned int>& outIndices);

void ComputeNormals(Mesh& mesh);