#pragma once

struct HalfEdge;

struct Face {
    /// <summary>
    /// An arbitrary edge that this face contains
    /// </summary>
    HalfEdge* edge = nullptr;
};