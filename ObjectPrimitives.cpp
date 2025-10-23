#include "ObjectPrimitives.h"


Mesh CreateCube()
{
    Mesh mesh;

    // 8 cube vertices
    auto v0 = mesh.addVertex({ -0.5f, -0.5f, -0.5f }); // back bottom left
    auto v1 = mesh.addVertex({ 0.5f, -0.5f, -0.5f }); // back bottom right
    auto v2 = mesh.addVertex({ 0.5f,  0.5f, -0.5f }); // back top right
    auto v3 = mesh.addVertex({ -0.5f,  0.5f, -0.5f }); // back top left

    auto v4 = mesh.addVertex({ -0.5f, -0.5f,  0.5f }); // front bottom left
    auto v5 = mesh.addVertex({ 0.5f, -0.5f,  0.5f }); // front bottom right
    auto v6 = mesh.addVertex({ 0.5f,  0.5f,  0.5f }); // front top right
    auto v7 = mesh.addVertex({ -0.5f,  0.5f,  0.5f }); // front top left

    // 6 faces (CCW when viewed from outside)
    mesh.addFace({ v4, v5, v6, v7 }); // Front (+Z)
    mesh.addFace({ v1, v0, v3, v2 }); // Back (-Z)
    mesh.addFace({ v0, v4, v7, v3 }); // Left (-X)
    mesh.addFace({ v5, v1, v2, v6 }); // Right (+X)
    mesh.addFace({ v3, v7, v6, v2 }); // Top (+Y)
    mesh.addFace({ v0, v1, v5, v4 }); // Bottom (-Y)

    return mesh;
}

//Mesh CreateCylinder(int resolution) {
//
//}