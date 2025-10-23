#pragma once

struct Vertex;
struct Face;

struct HalfEdge {
    Vertex* origin = nullptr;
    HalfEdge* next = nullptr;
    HalfEdge* twin = nullptr;
    Face* face = nullptr;
};