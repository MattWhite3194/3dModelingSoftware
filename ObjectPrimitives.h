#pragma once
#include "Mesh.h"

std::unique_ptr<Mesh> CreateCube(float size);

std::unique_ptr<Mesh> CreateCylinder(int resolution, float radius, float height);

std::unique_ptr<Mesh> CreateCone(int resolution, float radius, float height);

std::unique_ptr<Mesh> CreateCircle(int resolution, float radius);