#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal = glm::vec3(0.0f);
    /// <summary>
    /// An arbitrary half edge that this vertex is on
    /// </summary>
    HalfEdge* outgoing = nullptr;
};