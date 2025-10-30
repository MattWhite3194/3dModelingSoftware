#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <cmath>

inline float ImLength(const ImVec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline float ImDistance(const ImVec2& a, const ImVec2& b) {
    return ImLength(ImVec2(a.x - b.x, a.y - b.y));
}

inline ImVec2 ImNormalize(const ImVec2& v) {
    float len = ImLength(v);
    return len > 0.0f ? ImVec2(v.x / len, v.y / len) : ImVec2(0, 0);
}

inline float ImDot(const ImVec2& a, const ImVec2& b) {
    return a.x * b.x + a.y * b.y;
}