#include "ObjectPrimitives.h"


Mesh GetCubePrimitive() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 positions[] = {
        // Front face
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        // Back face
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
    };

    glm::vec3 normals[] = {
        { 0.0f,  0.0f,  1.0f},  // Front
        { 0.0f,  0.0f, -1.0f},  // Back
        { 0.0f,  1.0f,  0.0f},  // Top
        { 0.0f, -1.0f,  0.0f},  // Bottom
        { 1.0f,  0.0f,  0.0f},  // Right
        {-1.0f,  0.0f,  0.0f},  // Left
    };

    glm::vec2 texCoords[] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    glm::vec3 barycentrics[3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    for (int face = 0; face < 6; ++face)
    {
        glm::vec3 n = normals[face];
        Vertex v[4]{};

        switch (face) {
        case 0: // Front
            v[0].Position = positions[0];
            v[1].Position = positions[1];
            v[2].Position = positions[2];
            v[3].Position = positions[3];
            break;
        case 1: // Back
            v[0].Position = positions[5];
            v[1].Position = positions[4];
            v[2].Position = positions[7];
            v[3].Position = positions[6];
            break;
        case 2: // Top
            v[0].Position = positions[3];
            v[1].Position = positions[2];
            v[2].Position = positions[6];
            v[3].Position = positions[7];
            break;
        case 3: // Bottom
            v[0].Position = positions[4];
            v[1].Position = positions[5];
            v[2].Position = positions[1];
            v[3].Position = positions[0];
            break;
        case 4: // Right
            v[0].Position = positions[1];
            v[1].Position = positions[5];
            v[2].Position = positions[6];
            v[3].Position = positions[2];
            break;
        case 5: // Left
            v[0].Position = positions[4];
            v[1].Position = positions[0];
            v[2].Position = positions[3];
            v[3].Position = positions[7];
            break;
        }

        // Assign normal and UVs for the 4 vertices
        for (int i = 0; i < 4; ++i) {
            v[i].Normal = n;
            v[i].TexCoords = texCoords[i];
            v[i].Tangent = glm::vec3(1.0f);
            v[i].Bitangent = glm::vec3(1.0f);
            vertices.push_back(v[i]);
        }

        // Add indices for two triangles (per face)
        unsigned int startIndex = face * 4;
        indices.insert(indices.end(), {
            startIndex + 0, startIndex + 1, startIndex + 2,
            startIndex + 2, startIndex + 3, startIndex + 0
            });
    }
    Mesh primitiveCube(vertices, indices, {});
    return primitiveCube;
}