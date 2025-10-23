#pragma once

struct Vertex;
struct Face;

struct HalfEdge {
    /// <summary>
    /// The vertex this half-edge starts on.
    /// </summary>
    Vertex* origin = nullptr;
    /// <summary>
    /// The next halfedge this one points to going in a counter clockwise direction
    /// </summary>
    HalfEdge* next = nullptr;
    /// <summary>
    /// The half edge on the face on the opposite side
    /// </summary>
    HalfEdge* twin = nullptr;
    /// <summary>
    /// The face this half edge is connected to
    /// </summary>
    Face* face = nullptr;
};