#pragma once
#include "Mesh.h"

Mesh CreateCube(float size);

Mesh CreateCylinder(int resolution, float radius, float height);

Mesh CreateCone(int resolution, float radius, float height);

Mesh CreateCircle(int resolution, float radius);