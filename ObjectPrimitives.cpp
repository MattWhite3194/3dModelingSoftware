#include "ObjectPrimitives.h"
#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"


Mesh CreateCube(float size)
{
    Mesh mesh;
    float halfSize = size / 2.0f;
    // 8 cube vertices
    auto v0 = mesh.addVertex({ -halfSize, -halfSize, -halfSize }); // back bottom left
    auto v1 = mesh.addVertex({ halfSize, -halfSize, -halfSize }); // back bottom right
    auto v2 = mesh.addVertex({ halfSize,  halfSize, -halfSize }); // back top right
    auto v3 = mesh.addVertex({ -halfSize,  halfSize, -halfSize }); // back top left

    auto v4 = mesh.addVertex({ -halfSize, -halfSize,  halfSize }); // front bottom left
    auto v5 = mesh.addVertex({ halfSize, -halfSize,  halfSize }); // front bottom right
    auto v6 = mesh.addVertex({ halfSize,  halfSize,  halfSize }); // front top right
    auto v7 = mesh.addVertex({ -halfSize,  halfSize,  halfSize }); // front top left

    // 6 faces (CCW when viewed from outside)
    mesh.addFace({ v4, v5, v6, v7 }); // Front (+Z)
    mesh.addFace({ v1, v0, v3, v2 }); // Back (-Z)
    mesh.addFace({ v0, v4, v7, v3 }); // Left (-X)
    mesh.addFace({ v5, v1, v2, v6 }); // Right (+X)
    mesh.addFace({ v3, v7, v6, v2 }); // Top (+Y)
    mesh.addFace({ v0, v1, v5, v4 }); // Bottom (-Y)

    return mesh;
}

Mesh CreateCylinder(int resolution, float radius, float height) {
    Mesh mesh;
    std::vector<Vertex*> topVertices, bottomVertices;
    topVertices.reserve(resolution * sizeof(Vertex*));
    bottomVertices.reserve(resolution * sizeof(Vertex*));
    float angleFactor = glm::two_pi<float>() * (1.0f / resolution);
    //create a circle of vertices in hopefully the ccw direction
    for (int i = 0; i < resolution; i++) {
        float angle = angleFactor * i;
        auto v = mesh.addVertex({ radius * glm::sin(angle), radius * glm::cos(angle), -height / 2.0f});
        topVertices.push_back(v);
        angle = glm::two_pi<float>() - angleFactor * i;
        v = mesh.addVertex({ radius * glm::sin(angle), radius * glm::cos(angle), height / 2.0f });
        bottomVertices.push_back(v);
    }
    mesh.addFace(topVertices);
    mesh.addFace(bottomVertices);
    for (int i = 0; i < resolution; i++) {
        int nexTopIndex = (i + 1) % resolution;
        int bottomIndex = resolution - 1 - i;
        int nextBottomIndex = (bottomIndex + 1) % resolution;
        mesh.addFace({ topVertices[nexTopIndex], topVertices[i], bottomVertices[nextBottomIndex], bottomVertices[bottomIndex] });
    }
    return mesh;
}

Mesh CreateCone(int resolution, float radius, float height) {
    Mesh mesh;
    std::vector<Vertex*> vertices;
    vertices.reserve(resolution * sizeof(Vertex*));
    float angleFactor = glm::two_pi<float>() * (1.0f / resolution);
    //create a circle of vertices in hopefully the ccw direction
    for (int i = 0; i < resolution; i++) {
        float angle = glm::two_pi<float>() - angleFactor * i;
        auto v = mesh.addVertex({ radius * glm::sin(angle), radius * glm::cos(angle), -height / 2.0f });
        vertices.push_back(v);
    }
    mesh.addFace(vertices);
    Vertex* top = mesh.addVertex({ 0.0f, 0.0f, height / 2.0f });
    for (int i = 0; i < vertices.size(); i++) {
        mesh.addFace({ vertices[i], top, vertices[(i + 1) % resolution] });
    }
    return mesh;
}

Mesh CreateCircle(int resolution, float radius) {
    Mesh mesh;
    std::vector<Vertex*> vertices;
    vertices.reserve(resolution * sizeof(Vertex*));
    float angleFactor = glm::two_pi<float>() * (1.0f / resolution);
    //create a circle of vertices in hopefully the ccw direction
    for (int i = 0; i < resolution; i++) {
        float angle = angleFactor * i;
        auto v = mesh.addVertex({ radius * glm::sin(angle), radius * glm::cos(angle), 0.0f });
        vertices.push_back(v);
    }
    mesh.addFace(vertices);
    return mesh;
}